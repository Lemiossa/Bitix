/**
* utils.c
* Created by Matheus Leme Da Silva
*/
#include <types.h>
#include <x86.h>

/**
* Change the chacters attributes
*/
void setcolor(uchar bg, uchar fg)
{
  current_attr=((bg&0x07)<<4)|(fg&0x0f);
}

/**
* Puts string in the screen
*/
static uchar ansi_to_vga_color(uchar ansi)
{
  static uchar table[8]={0, 4, 2, 6, 1, 5, 3, 7};
  return (ansi<8)?table[ansi]:7;
}
void puts(uchar *str)
{
  enum { STATE_TEXT, STATE_ESC, STATE_CSI } state=STATE_TEXT;
  uchar params[8];
  int param_count=0;
  int cur_param=0;

  while(*str) {
    uchar c=*str++;

    switch(state) {
      case STATE_TEXT: {
        if(c==0x1b) {
          state=STATE_ESC;
        } else {
          putc(c);
        }
      } break;
      case STATE_ESC: {
        int i;
        if(c=='[') {
          state=STATE_CSI;
          param_count=0;
          cur_param=0;
          for(i=0;i<8;i++)
            params[i]=0;
        } else {
          state=STATE_TEXT;
        }
      } break;
      case STATE_CSI: {
        int i;
        if(c>='0'&&c<='9') {
          cur_param=cur_param*10+(c-'0');
        } else if(c==';') {
          if(param_count<8)
            params[param_count++]=cur_param;
          cur_param=0;
        } else if(c=='m') {
          if(param_count<8)
            params[param_count++]=cur_param;

          for(i=0;i<param_count;i++) {
            uchar p=params[i];
            if(p==0) {
              setcolor(0, 7); /* Reset color */
              current_attr&=~0x80; /* Disable blink */
            } else if(p>=30&&p<=37) {
              uchar fg=ansi_to_vga_color(p-30);
              current_attr=(current_attr&0xf8)|(fg&0x0f);
            } else if(p>=40&&p<=47) {
              uchar bg=ansi_to_vga_color(p-40);
              current_attr=(current_attr&0x8f)|((bg&0x07)<<4);
            } else if(p==5) {
              current_attr|=0x80; /* Enable blink */
            } else if(p==25) {
              current_attr&=~0x80; /* Disable blink */
            }
          }
          state=STATE_TEXT;
        } else {
          state=STATE_TEXT;
        }
      } break;
    }
  }
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

      while(*format>='0'&&*format<='9') {
        width=width*10+(*format-'0');
        format++;
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
      numtostr(val, buf, base, neg);
      if(width) putsp(buf, width, pad_char, width_neg);
      else puts(buf);
    } else {
      putc(*format++);
    }
  }
}

uint streq(uchar *a, uchar *b)
{
  while(*a&&(*a==*b)) {
    a++;
    b++;
  }
  return (*a==*b);
}

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
