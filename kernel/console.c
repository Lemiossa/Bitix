/**
 * console.c 
 * Criado por Matheus Leme Da Silva
 */
#include "console.h"
#include "ports.h"
#include "types.h"

#define WIDTH 80 
#define HEIGHT 25 

static uint8_t cursor_x=0;
static uint8_t cursor_y=0;
static uint8_t attr=0x07;

static uint16_t *vidmem=(uint16_t*)0xb8000;

void update_cursor()
{
	uint16_t pos=cursor_y*WIDTH+cursor_x;
	outb(0x3d4, 0x0f);
	outb(0x3d5, (uint8_t)(pos&0xff));
	outb(0x3d4, 0x0e);
	outb(0x3d5, (uint8_t)((pos>>8)&0xff));
}

void init_console()
{
	uint16_t pos=0;
	outb(0x3d4, 0xf);
	pos|=inb(0x3d5);

	outb(0x3d4, 0xe);
	pos|=((uint16_t)inb(0x3D5))<<8;

	cursor_x=pos%WIDTH;
	cursor_y=pos/WIDTH;
}

void scroll()
{
	if(cursor_y>=HEIGHT) {
		for(int y=1;y<HEIGHT;y++) {
			for(int x=0;x<WIDTH;x++) {
				vidmem[(y-1)*WIDTH+x]=vidmem[y*WIDTH+x];
			}
		}
		for(int x=0;x<WIDTH;x++) {
			vidmem[(HEIGHT-1)*WIDTH+x]=(attr<<8)|' ';
		}
		cursor_y=HEIGHT-1;
	}
}

void putc(char c)
{
	if(c=='\n') {
		cursor_x=0;
		cursor_y++;
	} else if(c=='\r') {
		cursor_x=0;
	} else {
		vidmem[cursor_y*WIDTH+cursor_x]=(attr<<8)|c;
		cursor_x++;
		if(cursor_x>=WIDTH){
			cursor_x=0;
			cursor_y++;
		}
	}
	scroll();
	update_cursor();
}

void puts(const char *s)
{
	while(*s)
		putc(*s++);
}

static void print_num_u64(uint64_t num,int base,int is_upper,int padc,int width)
{
	char buf[32];
	const char* digits=is_upper?"0123456789ABCDEF":"0123456789abcdef";
	int i=0;

	if(num==0) buf[i++]='0';
	else while(num) {
		buf[i++]=digits[num%base];
		num/=base;
	}

	while(i<width)putc(padc),width--;
	while(i--) putc(buf[i]);
}

static void print_signed(int64_t val,int base,int is_upper,int padc,int width)
{
	if(val<0) putc('-'),val=-val;
	print_num_u64((uint64_t)val,base,is_upper,padc,width);
}


void printf(const char* fmt, ...)
{
	va_list args;
	va_start(args,fmt);

	while(*fmt) {
		if(*fmt=='%') {
			fmt++;
			int padc=' ',width=0;
			if(*fmt=='0') padc='0',fmt++;
			while(*fmt>='0'&&*fmt<='9')
				width=width*10+(*fmt++-'0');


			int is_long=0,is_longlong=0,is_short=0,is_char=0;
			if(*fmt=='l') {
				fmt++;
				if(*fmt=='l')fmt++,is_longlong=1;
				else is_long=1;
			} else if(*fmt=='h') {
				fmt++;
				if(*fmt=='h')fmt++,is_char=1;
				else is_short=1;
			}

			switch(*fmt) {
				case 'd':
				case 'i': {
					int64_t val;
					if(is_longlong) val=va_arg(args,int64_t);
					else if(is_long) val=va_arg(args,long);
					else val=va_arg(args,int);
					if(is_short) val=(short)val;
					if(is_char) val=(char)val;
					print_signed(val,10,0,padc,width);
					break;
				}
				case 'u': {
					uint64_t val;
					if(is_longlong) val=va_arg(args,uint64_t);
					else if(is_long) val=va_arg(args,unsigned long);
					else val=va_arg(args,unsigned int);
					if(is_short) val=(unsigned short)val;
					if(is_char) val=(unsigned char)val;
					print_num_u64(val,10,0,padc,width);
					break;
				}
				case 'x':
				case 'X': {
					uint64_t val;
					int up=(*fmt=='X');
					if(is_longlong) val=va_arg(args,uint64_t);
					else if(is_long) val=va_arg(args,unsigned long);
					else val=va_arg(args,unsigned int);
					if(is_short) val=(unsigned short)val;
					if(is_char) val=(unsigned char)val;
					print_num_u64(val,16,up,padc,width);
					break;
				}
				case 'c': {
					char c=(char)va_arg(args,int);
					putc(c);
					break;
				}
				case 's': {
					const char* s=va_arg(args,const char*);
					puts(s);
					break;
				}
				case '%': {
					putc('%');
					break;
				}
				default: {
					putc(*fmt);
					break;
				}
			}
			fmt++;
		} else {
			putc(*fmt++);
		}
	}
	va_end(args);
}
