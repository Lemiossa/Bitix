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
      ch=lread(0xb800, from);
      attr=lread(0xb800, from+1);
      lwrite(0xb800, to, ch);
      lwrite(0xb800, to+1, attr);
    }
  }
  for(x=0;x<screen_width;x++) {
    ushort off=((screen_height-1)*screen_width+x)*2;
    lwrite(0xb800, off, ' ');
    lwrite(0xb800, off+1, current_attr);
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
  if(video_mode!=1)
    return;
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
    lwrite(0xb800, offset, ' ');
    lwrite(0xb800, offset+1, current_attr);
  } else if(c=='\t') {
    do {
      pos=cursor_y*screen_width+cursor_x;
      offset=pos*2;
      lwrite(0xb800, offset, ' ');
      lwrite(0xb800, offset+1, current_attr);
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
    lwrite(0xb800, offset, c);
    lwrite(0xb800, offset+1, current_attr);
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
      lwrite(0xb800, offset, ' ');
      lwrite(0xb800, offset+1, attr);
    }
  }
  cursor_x=0;
  cursor_y=0;
  current_attr=attr;
  setcursor(cursor_x, cursor_y);
}

/**
* Enable VGA blink
*/
void vga_enable_blink()
{
  uchar val;

  outportb(0x3c4, 0x01);
  val=inportb(0x3c5);
  val&=~0x20; /* Clear bit 5 */
  outportb(0x3c5, val);

  inportb(0x3da);
  outportb(0x3c0, 0x10);
  val=inportb(0x3c1);
  inportb(0x3da);
  outportb(0x3c0, 0x10);
  outportb(0x3c0, val|0x08);
}

/**
* Disable VGA blink
*/
void vga_disable_blink()
{
  uchar val;

  outportb(0x3c4, 0x01);
  val=inportb(0x3c5);
  val|=0x20; /* Set bit 5 */
  outportb(0x3c5, val);

  inportb(0x3da);
  outportb(0x3c0, 0x10);
  val=inportb(0x3c1);
  inportb(0x3da);
  outportb(0x3c0, 0x10);
  outportb(0x3c0, val&~0x08);
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
  while(*a&&(*a==*b)) {
    a++;
    b++;
  }
  return (*a==*b);
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
  while(*s)size++;
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
* Set VGA VIDEO/TEXT mode
*/
void io_set_video_mode(uchar mode)
{
  regs16_t r;
  r.h.ah=0x00;
  r.h.al=mode;
  int86(0x10, &r, &r);
}

/**
* Set 80x50 TEXT mode
*/
void set80x50mode()
{
	regs16_t r;

	/* Set 80x25 mode */
	io_set_video_mode(0x03);

	/* Set 8x8 font */
	r.h.ah=0x11;
	r.h.al=0x12;
	r.h.bl=0x00;
	int86(0x10, &r, &r);

	/* Update BDA */
	lwrite(0x40, 0x84, 50);
	lwrite(0x40, 0x4a, 49);

	/* Update global VARs */
	screen_width=80;
	screen_height=50;
}

/**
* Put pixel in graphic mode
*/
void putpixel(ushort x, ushort y, uchar color)
{
  /* video_mode=2 - 320x200x256 VGA mode */
  if(video_mode==2) {
  	ushort offset=y*screen_width+x;
  	lwrite(0xa000, offset, color);
  } else if(video_mode==1) puts("ERROR: Attempted to draw a pixel in an incompatible mode\n");
  return;
}

/**
* Remap PIC
*/
void pic_remap()
{
  outportb(0x20, 0x11);
  outportb(0xa0, 0x11);
  outportb(0x21, 0x20);
  outportb(0xa1, 0x28);
  outportb(0x21, 0x04);
  outportb(0xa1, 0x02);
  outportb(0x21, 0x01);
  outportb(0xa1, 0x01);
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

/**
* Initialize disk
*/
void init_disk(uchar drive)
{
  io_init_disk(drive);
  switch(drive) {
    case 0x00: {
      disk.label[0]='f';
      disk.label[1]='d';
      disk.label[2]='0';
      disk.label[3]=0;
    } break;
    case 0x01: {
      disk.label[0]='f';
      disk.label[1]='d';
      disk.label[2]='1';
      disk.label[3]=0;
    } break;
    case 0x80: {
      disk.label[0]='h';
      disk.label[1]='d';
      disk.label[2]='0';
      disk.label[3]=0;
    } break;
    case 0x81: {
      disk.label[0]='h';
      disk.label[1]='d';
      disk.label[2]='1';
      disk.label[3]=0;
    } break;
    default: {
      disk.label[0]='u';
      disk.label[1]='n';
      disk.label[2]='k';
      disk.label[3]=0;
    } break;
  }
}

#define BLOCK 512
#define MAX_NAME 32
#define MAX_PATH 16

typedef struct {
  uchar magic[4];
  ushort total_blocks;
  ushort inode_start;
  ushort data_start;
  ushort inode_count;
  ushort used_inodes;
  ushort free_blocks;
  ushort used_blocks;
  ushort root_inode;
  ushort mount_count;
  ushort last_mount_time;
  ushort last_write_time;
  ushort checksum;
  uchar reserved[BLOCK-28];
} bfx_super_t;

typedef struct {
  uchar mode;
  ushort links;
  ushort size;
  ushort start;
  ushort created;
  ushort modified;
  ushort accessed;
  ushort parent;
  uchar reserved[15];
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
      lwrite(current_seg, current_off+j, tmpbuf[j]);
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
