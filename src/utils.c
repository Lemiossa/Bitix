/**
* utils.c
* Created by Matheus Leme Da Silva
*/
#include <types.h>
#include <x86.h>
#include <utils.h>

uchar cursor_x, cursor_y;

enum { STATE_TEXT, STATE_ESC, STATE_CSI };
static ansi_state=STATE_TEXT;
static uchar params[8];
static int param_count=0;
static int cur_param=0;

uchar current_attr=0x07;
uint screen_width=80;
uint screen_height=25;
uchar video_mode=1;

int shift_pressed=0;
int caps_lock=0;
int ctrl_pressed=0;

/**
* Change the chacters attributes
*/
void setcolor(uchar bg, uchar fg)
{
  current_attr=((bg&0x07)<<4)|(fg&0x0f);
}

/**
* Scroll the screen
*/
void scroll()
{
  ushort x, y, from, to;
  for(y=1;y<screen_height;y++) {
    for(x=0;x<screen_width;x++) {
      uchar ch, attr;
      from=(y*screen_width+x)*2;
      to=((y-1)*screen_width+x)*2;
      ch=lreadb(0xb800, from);
      attr=lreadb(0xb800, from+1);
      lwriteb(0xb800, to, ch);
      lwriteb(0xb800, to+1, attr);
    }
  }
  for(x=0;x<screen_width;x++) {
    ushort off=((screen_height-1)*screen_width+x)*2;
    lwriteb(0xb800, off, ' ');
    lwriteb(0xb800, off+1, current_attr);
  }
  if(cursor_y>0)
    cursor_y--;
}

/**
* Print char in the screen
*/
static void io_putc(uchar c)
{
  ushort pos, offset;
  if(c=='\n') {
    cursor_x=0;
    cursor_y++;
    if(cursor_y>=screen_height) {
      scroll();
    }
  } else if(c=='\r') {
    cursor_x=0;
  } else if(c=='\b') {
    if(cursor_x>0)
      cursor_x--;
    else if(cursor_y>0) {
      cursor_y--;
      cursor_x=screen_width-1;
    }
    pos=cursor_y*screen_width+cursor_x;
    offset=pos*2;
    lwriteb(0xb800, offset, ' ');
    lwriteb(0xb800, offset+1, current_attr);
  } else if(c=='\t') {
    do {
      pos=cursor_y*screen_width+cursor_x;
      offset=pos*2;
      lwriteb(0xb800, offset, ' ');
      lwriteb(0xb800, offset+1, current_attr);
      cursor_x++;
      if(cursor_x>=screen_width) {
        cursor_x=0;
        cursor_y++;
        if(cursor_y>=screen_height)
          scroll();
      }
    } while(cursor_x%4!=0);
  } else {
    pos=cursor_y*screen_width+cursor_x;
    offset=pos*2;
    lwriteb(0xb800, offset, c);
    lwriteb(0xb800, offset+1, current_attr);
    cursor_x++;
    if(cursor_x>=screen_width) {
      cursor_x=0;
      cursor_y++;
      if(cursor_y>=screen_height)
        scroll();
    }
  }
  setcursor(cursor_x, cursor_y);
}
static uchar ansi_to_vga_color(uchar ansi)
{
  static uchar table[8]={0, 4, 2, 6, 1, 5, 3, 7};
  return (ansi<8)?table[ansi]:7;
}
void putc(uchar c)
{
  switch(ansi_state) {
    case STATE_TEXT: {
      if(c==0x1b) {
        ansi_state=STATE_ESC;
      } else {
        io_putc(c);
      }
    } break;
    case STATE_ESC: {
      if(c=='[') {
        int i;
        ansi_state=STATE_CSI;
        param_count=0;
        cur_param=0;
        for(i=0;i<8;i++)
          params[i]=0;
      } else {
        ansi_state=STATE_TEXT;
        putc(0x1b);
        putc(c);
      }
    } break;
    case STATE_CSI: {
      if(c>='0'&&c<='9') {
        cur_param=cur_param*10+(c-'0');
      } else if(c==';') {
        if(param_count<8)
          params[param_count++]=cur_param;
        cur_param=0;
      } else if(c=='m') {
        int i;
        if(param_count<8)
          params[param_count++]=cur_param;
        for(i=0;i<param_count;i++) {
          uchar p=params[i];
          if(p==0) {
            setcolor(0, 7);
            current_attr&=~0x80;
          } else if(p>=30&&p<=37) {
            uchar fg=ansi_to_vga_color(p-30);
            current_attr=(current_attr&0xf8)|(fg&0x0f);
          } else if(p>=40&&p<=47) {
            uchar bg=ansi_to_vga_color(p-40);
            current_attr=(current_attr&0x8f)|((bg&0x07)<<4);
          } else if(p==5) {
            current_attr|=0x80;
          } else if(p==25) {
            current_attr&=~0x80;
          }
        }
        ansi_state=STATE_TEXT;
      } else {
        ansi_state=STATE_TEXT;
      }
    } break;
  }
}

/**
* Puts string in the screen
*/
void puts(uchar *str)
{
  while(*str) putc(*str++);
}

/**
* Clear screen with attribute
*/
void clear_screen(uchar attr)
{
  ushort x, y, offset;
  for(y=0;y<screen_height;y++) {
    for(x=0;x<screen_width;x++) {
      offset=(y*screen_width+x)*2;
      lwriteb(0xb800, offset, ' ');
      lwriteb(0xb800, offset+1, attr);
    }
  }
  cursor_x=0;
  cursor_y=0;
  current_attr=attr;
  setcursor(cursor_x, cursor_y);
}

/**
* Convert number to string
*/
char *numtostr(ulong val, uchar *out, ushort base, bool neg)
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

/**
* Put string with padding
*/
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

/**
* Put string formatted
*/
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

      if(*format=='*'){
      	width=*((uint*)arg++);
      	format++;
      } else {
      	while(*format>='0'&&*format<='9') {
          width=width*10+(*format-'0');
          format++;
        }
      }

      if(*format=='l') {
        val=*((ulong*)arg);
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
          val=(ulong)(-(slong)val)+1;
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
      numtostr(val, buf, base, neg);
      if(width) putsp(buf, width, pad_char, width_neg);
      else puts(buf);
    } else {
      putc(*format++);
    }
  }
}

/**
* Compare strings
*/
uint streq(uchar *a, uchar *b)
{
  while(*a&&*b){
  	if(*a!=*b) return 0;
  	a++;
  	b++;
  }
  return *a==*b;
}

/**
* Compare n bytes in strings
*/
uint strneq(uchar *a, uchar *b, uint n)
{
  while(n&&*a&&(*a==*b)) {
    a++;
    b++;
    n--;
  }
  if(n==0) return 1;
  return (*a==*b);
}

/**
* Return len of string
*/
uint strlen(uchar *s)
{
  int size=0;
  while(*s++)size++;
  return size;
}

/**
* Interrupt
*/
void int86(uchar intnum, regs16_t *in, regs16_t *out)
{
  switch(intnum) {
    case 0x10: {
      setregs(in);
      #asm
      int 0x10
      #endasm
    } break;
    case 0x11: {
      setregs(in);
      #asm
      int 0x11
      #endasm
    } break;
    case 0x12: {
      setregs(in);
      #asm
      int 0x12
      #endasm
    } break;
    case 0x13: {
      setregs(in);
      #asm
      int 0x13
      #endasm
    } break;
    case 0x14: {
      setregs(in);
      #asm
      int 0x14
      #endasm
    } break;
    case 0x15: {
      setregs(in);
      #asm
      int 0x15
      #endasm
    } break;
    default: {
      puts("Error: unknown interrupt!\n");
    } break;
  }
  getregs(out);
}

/**
* Put pixel in graphic mode
*/
void putpixel(ushort x, ushort y, uchar color)
{
  /* video_mode=2 - 320x200x256 VGA mode */
  if(video_mode==2) {
  	ushort offset=y*screen_width+x;
  	lwriteb(0xa000, offset, color);
  } else if(video_mode==1) puts("ERROR: Attempted to draw a pixel in an incompatible mode\n");
  return;
}

/**
* Read block using CHS
*/
#define MAX_DISK_RETRIES 3
int readblock(ushort lba, void *buf)
{
  ushort spt=disk.spt;
  ushort heads=disk.heads;
  ushort cylinder=lba/(spt*heads);
  ushort temp=lba%(spt*heads);
  uchar head=temp/spt;
  ushort sector=(temp%spt)+1;
  int i, result;

  for(i=0;i<=MAX_DISK_RETRIES;i++) {
    result=io_readblock_chs(head, cylinder, sector, buf);
    if(result)reset_disk();
    else break;
  }

  return result;
}

/**
* Set cursor position
*/
void setcursor(uchar x, uchar y)
{
  ushort pos=y*screen_width+x;
  if(video_mode!=1)return;
  outportb(0x3d4, 0x0e);
  outportb(0x3d5, (pos>>8)&0xff);
  outportb(0x3d4, 0x0f);
  outportb(0x3d5, pos&0xff);
}

/**
* Dump a buffer
*/
void hexdump(void *data, uint size)
{
  uchar *buffer=(uchar*)data;
  uint i, j;
  uint bytes_per_line=(screen_width-10)/4;
  if(bytes_per_line==0)
    bytes_per_line=1;
  kputsf("%-4s | %-*s | %-*s\n", "OFF", bytes_per_line*3, "HEX", bytes_per_line, "ASCII");
  for(i=0;i<screen_width-2;i++) {
    putc('-');
  }
  putc('\n');
  for(i=0;i<size;i+=bytes_per_line) {
    kputsf("%{0}04x |", i);
    for(j=0;j<bytes_per_line;j++) {
      if(i+j<size)
        kputsf(" %{0}02x", buffer[i+j]);
      else
        puts("   ");
    }
    puts("  | ");
    for(j=0;j<bytes_per_line;j++) {
      if(i+j<size) {
        uchar c=buffer[i+j];
        if(c>=32&&c<=126)
          putc(c);
        else
          putc('.');
      } else {
        putc(' ');
      }
    }
    puts("\n");
  }
}

#define BLOCK 512
#define MAX_NAME 22
#define MAX_PATH 16

typedef struct {
  uchar magic[4];
  ushort total_blocks;
  ushort inode_start;
  ushort data_start;
  ushort inode_count;
  ushort used_inodes;
  ushort free_blocks;
  ushort root_inode;
  uchar reserved[BLOCK-(4+2*7)];
} bfx_super_t;

typedef struct {
  uchar mode;
  ushort size;
  ushort start;
  ushort created;
  ushort parent;
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

/**
* Copy bytes to buffer
*/
void copy_bytes(void *dst, void *src, int n)
{
  uchar *d=(uchar*)dst;
  uchar *s=(uchar*)src;
  int i;
  for(i=0;i<n;i++) {
    d[i]=s[i];
  }
}

/**
* Mount BFX filesystem
*/
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

/**
* Read a inode with index
*/
int read_inode(ushort idx, bfx_inode_t *out)
{
  ushort block, offset;
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

/**
* Compare BFX filenames
*/
int name_match(bfx_inode_t *inode, uchar *name)
{
  int i;
  for(i=0;i<MAX_NAME;i++) {
    if(name[i]==0&&inode->name[i]==0)return 1;
    if(name[i]!=inode->name[i])return 0;
  }
  return 0;
}

/**
* Parse PATH
*/
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

/**
* Find inode in directory
*/
ushort find_inode_in_dir(ushort dir_idx, uchar *name)
{
  bfx_inode_t dir;
  ushort pos=0;
  if(read_inode(dir_idx, &dir))return 0;
  if(!(dir.mode&0x80)) return 0;
  while(pos<dir.size) {
    ushort block_idx=dir.start+pos/BLOCK;
    ushort offset=pos%BLOCK;
    ushort  child_idx=0;
    bfx_inode_t child;
    if(block_idx>=sb.total_blocks) return 0;
    if(readblock(block_idx, tmpbuf)) return 0;
    child_idx=*((ushort*)(tmpbuf+offset));
    if(child_idx==0)return 0;
    if(read_inode(child_idx, &child)) return 0;
    if(name_match(&child, name)) return child_idx;
    pos+=sizeof(ushort);
  }
  return 0;
}

/**
* Resolve path
*/
ushort resolve_path(uchar *path)
{
  uchar components[MAX_PATH][MAX_NAME];
  int count=parse_path(path, components);
  ushort current;
  int i;
  if(count<0) return 0;
  current=sb.root_inode;
  for(i=0;i<count;i++) {
    current=find_inode_in_dir(current, components[i]);
    if(current==0) return 0;
  }
  return current;
}

/**
* Load file in BFX
*/
int bfx_readfile(uchar *path, ushort seg, ushort off)
{
  ushort idx;
  bfx_inode_t file;
  ushort size;
  ushort start_block;
  ushort bytes_left;
  ushort current_seg, current_off;
  ushort i;
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
    ushort to_copy, j;
    if(start_block+i>=sb.total_blocks) {
      kputsf("readfile: Invalid block %u\r\n", start_block+i);
      return ERR_IO;
    }
    if(readblock(start_block+i, tmpbuf)) return ERR_IO;
    to_copy=bytes_left>BLOCK?BLOCK:bytes_left;
    for(j=0;j<to_copy;j++) {
      lwriteb(current_seg, current_off+j, tmpbuf[j]);
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

/**
* Find char in string
*/
char *strchr(uchar *s, int c)
{
  while(*s) {
    if(*s==(uchar)c)
      return (uchar*)s;
    s++;
  }
  return NULL;
}

/**
* Find string in string
*/
char *strstr(uchar *haystack, uchar *needle)
{
  if(!*needle) return (uchar*)haystack;

  while(*haystack) {
    uchar *h=haystack;
    uchar *n=needle;

    while(*h&&*n&&*h==*n)  {
      h++;
      n++;
    }

    if(!*n) return (uchar*)haystack;
    haystack++;
  }
}

/**
* Copy long memory
*/
void lmemcpy(void *buf, ushort seg, ushort off, uint size)
{
  int i;
  uchar *out=(uchar*)buf;
  for(i=0;i<size;i++) {
    out[i]=lreadb(seg, off+i);
  }
}

/**
* Exec file
*/
#ifndef PROG
#define USERSEG 0x3000
#else
#define USERSEG 0x5000
#endif
#define USEROFF 0x0000
int exec(uchar *path)
{
  int result;
  result=bfx_readfile(path, USERSEG, USEROFF);
  if(result<0) {
    kputsf("exec: Error in execute '%s'\n", path);
    return result;
  }
  lcall();
}

#define CTRL_CHAR(c) ((c)&0x1f)

uint getkey()
{
  uchar code=io_get_key();

  if(code&0x80) {
    uchar released=code&0x7f;
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
    case 0x1c: return '\r';
    case 0x0e: return '\b';
    case 0x0f: return '\t';

    default:   return 0;
  }
}

#define MAX_LINE 256

int readline(uchar *buffer, int max_len)
{
  int pos=0;
  while(1) {
  	uint key=getkey();
  	if(key==0) continue;
  	if(key=='\r') {
      putc('\n');
      buffer[pos]=0;
      return pos;
  	} else if(key=='\b') {
      if(pos>0) {
        pos--;
        putc('\b');
        putc(' ');
        putc('\b');
      }
    } else {
      if(pos<max_len-1) {
        buffer[pos++]=(uchar)key;
        putc((uchar)key);
      }
    }
  }
}
