/**
* utils.c
* Created by Matheus Leme Da Silva
*/
#include "types.h"
#include "utils.h"
#include "x86.h"

int shift_pressed=0;
int caps_lock=0;
int ctrl_pressed=0;

void ntos(u32 val, uchar *out, u16 base, bool neg)
{
  char tmp[32];
  char *digits="0123456789abcdef";
  int i=0, j=0;
  if(val==0) {
    out[0]='0';
    out[1]='\0';
    return;
  }
  while(val&&i<31) {
    tmp[i++]=digits[val%base];
    val/=base;
  }
  if(neg)tmp[i++]='-';
  while(i--)out[j++]=tmp[i];
  out[j]=0;
}

void putcat(uchar c, uint x, uint y, uchar attr)
{
  uint offset=(y*80+x)*2;
  u16 val=((u16)attr<<8)|c;
  lwrite16(0xb800, offset, val);
}

void puts(uchar *s)
{
  while(*s) {
    putc(*s++);
  }
}

void putsp(uchar *s, int width, uchar pad_char, bool neg)
{
  int len=0;
  int pad;
  while(s[len])len++;
  pad=(width>0)?(width-len):(-width-len);
  if(pad<0)pad=0;
  if(!neg) {
    while(pad-->0)putc(pad_char);
    puts(s);
  } else {
    puts(s);
    while(pad-->0)putc(pad_char);
  }
}

#define __max_buf_size 32
void kputsf(uchar *format, ...)
{
  uint *arg=(uint*)(&format+1);
  while(*format) {
    if(*format=='%') {
      uchar buf[__max_buf_size];
      long val;
      ulong uval;
      long neg;
      bool is_long=false;
      uint base=10;
      int width=0;
      bool width_neg=false;
      uchar pad_char=' ';

      format++;

      if(*format=='{'&&format[2]=='}') {
        pad_char=format[1];
        format+=3;
      }

      if(*format=='-') {
        width_neg=true;
        width=-width;
        format++;
      }

      while(*format>='0'&&*format<='9') {
        width=width*10+(*format-'0');
        format++;
      }

      if(*format=='l') {
        val=*((u32*)arg);
        arg+=2;
        is_long=true;
        format++;
      } else {
        val=*((uint*)arg++);
        is_long=false;
      }
      uval=val;
      neg=((val&(is_long?0x80000000:0x8000))?true:false);
      if(neg) {
        if(is_long) {
          val=(~val)+1;
        } else {
          val=(u32)(-(s32)val)+1;
        }
      }

      if(*format=='d'||*format=='i') {
        base=10;
      } else if(*format=='u') {
        base=10;
        neg=false;
        val=uval;
      } else if(*format=='x') {
        base=16;
        val=uval;
      } else if(*format=='b') {
        base=2;
      } else if(*format=='o') {
        base=8;
      } else if(*format=='%') {
        puts("%");
        format++;
        continue;
      } else if(*format=='s') {
        uchar *str=val;
        if(str){
          if(width)putsp(str, width, ' ', width_neg);
          else puts(str);
        }
        format++;
        continue;
      } else {
      	format++;
      	continue;
      }
      format++;
      ntos(val, buf, base, neg);
      if(width) putsp(buf, width, pad_char, width_neg);
      else puts(buf);
    } else {
      putc(*format++);
    }
  }
}

#define MAX_DISK_RETRIES 3
int readblock(u16 lba, void *buf)
{
  u16 spt=disk.spt;
  u16 heads=disk.heads;
  u16 cylinder=lba/(spt*heads);
  u16 temp=lba%(spt*heads);
  u8 head=temp/spt;
  u16 sector=(temp%spt)+1;
  int i, result;

  for(i=0;i<=MAX_DISK_RETRIES;i++) {
    result=io_readblock_chs(head, cylinder, sector, buf);
    if(result)reset_disk();
    else break;
  }

  return result;
}

#define BLOCK 512
#define MAX_NAME 32
#define MAX_PATH 16

typedef struct {
  uchar magic[4];
  u16 total_blocks;
  u16 inode_start;
  u16 data_start;
  u16 inode_count;
  u16 used_inodes;
  u16 free_blocks;
  u16 used_blocks;
  u16 root_inode;
  u16 mount_count;
  u16 last_mount_time;
  u16 last_write_time;
  u16 checksum;
  uchar reserved[BLOCK-28];
} bfx_super_t;

typedef struct {
  u8 mode;
  u16 links;
  u16 size;
  u16 start;
  u16 created;
  u16 modified;
  u16 accessed;
  u16 parent;
  u8 reserved[15];
  uchar name[MAX_NAME];
} bfx_inode_t;

#define BFX_SUPER_SEC 32
#define TMP_BUF_SIZE 512

#define INPB (uint)(BLOCK/sizeof(bfx_inode_t))

/* Errors */
#define ERR_IO            -1
#define ERR_NOTFOUND      -2
#define ERR_PERMDEN       -3
#define ERR_TOOLARGE      -4
#define ERR_NOMEM         -5
#define ERR_NOTFILE       -6
#define ERR_INVPATH	      -7

bfx_super_t sb;

uchar tmpbuf[TMP_BUF_SIZE];

void copy_bytes(void *dst, void *src, int n)
{
  uchar *d=(uchar*)dst;
  uchar *s=(uchar*)src;
  int i;
  for(i=0;i<n;i++) {
    d[i]=s[i];
  }
}

int bfx_mount()
{
  int i;
  if(readblock(BFX_SUPER_SEC, tmpbuf))return ERR_IO;
  copy_bytes((void*)&sb, tmpbuf, sizeof(bfx_super_t));
  if(sb.magic[0]!='B'||sb.magic[1]!='F'||sb.magic[2]!='X') {
    kputsf("mount: magic is invalid\r\n");
    return ERR_IO;
  }
  return 0;
}

int read_inode(u16 idx, bfx_inode_t *out)
{
  u16 block, offset;
  uchar *inode_ptr;
  int i;

  if(idx==0) return ERR_NOTFOUND;

  block=sb.inode_start+idx/INPB;
  offset=idx%INPB;

  if(block>=sb.total_blocks) {
  	kputsf("read_inode: Block is too large\r\n");
  	return ERR_IO;
  }
  if(readblock(block, tmpbuf)) {
  	kputsf("read_inode: failed to read block %u\r\n", block);
  	return ERR_IO;
  }
  inode_ptr=tmpbuf+offset*sizeof(bfx_inode_t);
  copy_bytes(out, inode_ptr, sizeof(bfx_inode_t));

  return 0;
}

int name_match(bfx_inode_t *inode, uchar *name)
{
  int i;
  for(i=0;i<MAX_NAME;i++) {
    if(name[i]==0&&inode->name[i]==0)return 1;
    if(name[i]!=inode->name[i])return 0;
  }
  return 0;
}

int parse_path(uchar *path, uchar components[MAX_PATH][MAX_NAME])
{
  int comp_count=0;
  int pos=0;
  int cpos=0;
  if(!path||path[0]!='/') return -1;
  pos++;
  while(path[pos]&&comp_count<MAX_PATH) {
    if(path[pos]=='/') {
      if(cpos>0) {
        components[comp_count][cpos]=0;
        comp_count++;
        cpos=0;
      }
      pos++;
      continue;
    }
    if(cpos<MAX_NAME-1) {
      components[comp_count][cpos++]=path[pos++];
    } else {
      return -1;
    }
  }
  if(cpos>0) {
    components[comp_count][cpos]=0;
    comp_count++;
  }
  return comp_count;
}

u16 find_inode_in_dir(u16 dir_idx, uchar *name)
{
  bfx_inode_t dir;
  u16 pos=0;
  if(read_inode(dir_idx, &dir))return 0;
  if(!(dir.mode&0x80)) return 0;
  while(pos<dir.size) {
    u16 block_idx=dir.start+pos/BLOCK;
    u16 offset=pos%BLOCK;
    u16 child_idx=0;
    bfx_inode_t child;
    if(block_idx>=sb.total_blocks) return 0;
    if(readblock(block_idx, tmpbuf)) return 0;
    child_idx=*((u16*)(tmpbuf+offset));
    if(child_idx==0)return 0;
    if(read_inode(child_idx, &child)) return 0;
    if(name_match(&child, name)) return child_idx;
    pos+=sizeof(u16);
  }
  return 0;
}

u16 resolve_path(uchar *path)
{
  uchar components[MAX_PATH][MAX_NAME];
  int count=parse_path(path, components);
  u16 current;
  int i;
  if(count<0) return 0;
  current=sb.root_inode;
  for(i=0;i<count;i++) {
    current=find_inode_in_dir(current, components[i]);
    if(current==0) return 0;
  }
  return current;
}

int bfx_readfile(uchar *path, u16 seg, u16 off)
{
  u16 idx;
  bfx_inode_t file;
  u16 size;
  u16 start_block;
  u16 bytes_left;
  u16 current_seg, current_off;
  u16 i;
  if(path[0]!='/') {
  	puts("readfile: Invalid path: %s\r\n", path);
  	return ERR_INVPATH;
  }
  idx=resolve_path(path);
  if(idx==0) {
    kputsf("readfile: No such file or directory: %s\r\n", path);
    return ERR_NOTFOUND;
  }
  if(read_inode(idx, &file)) return ERR_IO;
  if(file.mode&0x80) {
    kputsf("readfile: Not a file: %s\r\n", path);
    return ERR_NOTFILE;
  }

  size=file.size;
  start_block=file.start;
  bytes_left=size;
  current_seg=seg;
  current_off=off;

  for(i=0;i<(size+BLOCK-1)/BLOCK;i++) {
    u16 to_copy, j;
    if(start_block+i>=sb.total_blocks) {
      kputsf("readfile: Invalid block %u\r\n", start_block+i);
      return ERR_IO;
    }
    if(readblock(start_block+i, tmpbuf)) return ERR_IO;
    to_copy=bytes_left>BLOCK?BLOCK:bytes_left;
    for(j=0;j<to_copy;j++) {
      lwrite8(current_seg, current_off+j, tmpbuf[j]);
    }
    current_off+=BLOCK;
    if(current_off>=0x10000) {
      current_seg+=0x1000;
      current_off-=0x10000;
    }
    bytes_left-=to_copy;
  }
  return 0;
}

int cmpstr(uchar *s1, uchar *s2)
{
  while (*s1 && (*s1 == *s2)) {
    s1++;
    s2++;
  }
  return *(uchar*)s1-*(uchar*)s2;
}

int lenofstr(uchar *str) {
  int i=0;
  while(*str++)i++;
  return i;
}

#define CTRL_CHAR(c) ((c)&0x1f)

uint getkey()
{
  u8 code=io_get_key();

  if(code&0x80) {
    u8 released=code&0x7f;
    if(released==0x2a||released==0x36) shift_pressed=0;
    if(released==0x1d) ctrl_pressed=0;
    return 0;
  }

  if(code==0x2a||code==0x36) {
    shift_pressed=1;
    return 0;
  }

  if(code==0x3a) {
    caps_lock^=1;
    return 0;
  }

  if(code==0x1d) {
    ctrl_pressed=1;
    return 0;
  }

  switch(code) {
    case 0x01: return KEY_ESC;
    case 0x02: return shift_pressed?'!':'1';
    case 0x03: return shift_pressed?'@':'2';
    case 0x04: return shift_pressed?'#':'3';
    case 0x05: return shift_pressed?'$':'4';
    case 0x06: return shift_pressed?'%':'5';
    case 0x07: return shift_pressed?'^':'6';
    case 0x08: return shift_pressed?'&':'7';
    case 0x09: return shift_pressed?'*':'8';
    case 0x0a: return shift_pressed?'(':'9';
    case 0x0b: return shift_pressed?')':'0';
    case 0x0c: return shift_pressed?'_':'-';
    case 0x0d: return shift_pressed?'+':'=';

    case 0x1a: return shift_pressed?'{':'[';
    case 0x1b: return shift_pressed?'}':']';
    case 0x2b: return shift_pressed?'\\':'|';
    case 0x27: return shift_pressed?':':';';
    case 0x28: return shift_pressed?'"':'\'';
    case 0x29: return shift_pressed?'~':'`';
    case 0x33: return shift_pressed?'<':',';
    case 0x34: return shift_pressed?'>':'.';
    case 0x35: return shift_pressed?'?':'/';

    case 0x10: return ctrl_pressed?CTRL_CHAR('Q'):(shift_pressed^caps_lock)?'Q':'q';
    case 0x11: return ctrl_pressed?CTRL_CHAR('W'):(shift_pressed^caps_lock)?'W':'w';
    case 0x12: return ctrl_pressed?CTRL_CHAR('E'):(shift_pressed^caps_lock)?'E':'e';
    case 0x13: return ctrl_pressed?CTRL_CHAR('R'):(shift_pressed^caps_lock)?'R':'r';
    case 0x14: return ctrl_pressed?CTRL_CHAR('T'):(shift_pressed^caps_lock)?'T':'t';
    case 0x15: return ctrl_pressed?CTRL_CHAR('Y'):(shift_pressed^caps_lock)?'Y':'y';
    case 0x16: return ctrl_pressed?CTRL_CHAR('U'):(shift_pressed^caps_lock)?'U':'u';
    case 0x17: return ctrl_pressed?CTRL_CHAR('I'):(shift_pressed^caps_lock)?'I':'i';
    case 0x18: return ctrl_pressed?CTRL_CHAR('O'):(shift_pressed^caps_lock)?'O':'o';
    case 0x19: return ctrl_pressed?CTRL_CHAR('P'):(shift_pressed^caps_lock)?'P':'p';
    case 0x1e: return ctrl_pressed?CTRL_CHAR('A'):(shift_pressed^caps_lock)?'A':'a';
    case 0x1f: return ctrl_pressed?CTRL_CHAR('S'):(shift_pressed^caps_lock)?'S':'s';
    case 0x20: return ctrl_pressed?CTRL_CHAR('D'):(shift_pressed^caps_lock)?'D':'d';
    case 0x21: return ctrl_pressed?CTRL_CHAR('F'):(shift_pressed^caps_lock)?'F':'f';
    case 0x22: return ctrl_pressed?CTRL_CHAR('G'):(shift_pressed^caps_lock)?'G':'g';
    case 0x23: return ctrl_pressed?CTRL_CHAR('H'):(shift_pressed^caps_lock)?'H':'h';
    case 0x24: return ctrl_pressed?CTRL_CHAR('J'):(shift_pressed^caps_lock)?'J':'j';
    case 0x25: return ctrl_pressed?CTRL_CHAR('K'):(shift_pressed^caps_lock)?'K':'k';
    case 0x26: return ctrl_pressed?CTRL_CHAR('L'):(shift_pressed^caps_lock)?'L':'l';
    case 0x2c: return ctrl_pressed?CTRL_CHAR('Z'):(shift_pressed^caps_lock)?'Z':'z';
    case 0x2d: return ctrl_pressed?CTRL_CHAR('X'):(shift_pressed^caps_lock)?'X':'x';
    case 0x2e: return ctrl_pressed?CTRL_CHAR('C'):(shift_pressed^caps_lock)?'C':'c';
    case 0x2f: return ctrl_pressed?CTRL_CHAR('V'):(shift_pressed^caps_lock)?'V':'v';
    case 0x30: return ctrl_pressed?CTRL_CHAR('B'):(shift_pressed^caps_lock)?'B':'b';
    case 0x31: return ctrl_pressed?CTRL_CHAR('N'):(shift_pressed^caps_lock)?'N':'n';
    case 0x32: return ctrl_pressed?CTRL_CHAR('M'):(shift_pressed^caps_lock)?'M':'m';

    case 0x48: return KEY_UP;
    case 0x50: return KEY_DOWN;
    case 0x4b: return KEY_LEFT;
    case 0x4d: return KEY_RIGHT;

    case 0x39: return ' ';
    case 0x1c: return '\n';
    case 0x0e: return '\b';
    case 0x0f: return '\t';

    default:   return 0;
  }
}

ulong get_ms() {
  return ticks*TICK_MS;
}

ulong get_secs() {
  return ticks/100;
}

void sleep_ms(uint ms) {
  u32 start=get_ms();
  while((get_ms()-start)<ms)hlt();
}

void putsxy(int x, int y, const uchar *s, uchar attr)
{
  int i = 0;
  while (s[i]) {
    putcat(s[i], x + i, y, attr);
    i++;
  }
}

void wait_input_buffer_empty()
{
  while(inb(0x64)&0x02);
}
void enable_a20()
{
  wait_input_buffer_empty();
  outb(0x64, 0xd1);

  wait_input_buffer_empty();
  outb(0x60, 0xdf);
}
