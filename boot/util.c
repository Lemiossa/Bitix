/**
 * util.c
 * Created by Matheus Leme Da Silva
 */
#include "types.h"
#include "x86.h"
#include "util.h"
#include "ports.h"

bool serial_initialized=false;

void serial_init()
{
	if(serial_initialized)
		return;
	outb(0x3f8+1, 0x00);
	outb(0x3f8+3, 0x80);
	outb(0x3f8+0, 0x03);
	outb(0x3f8+1, 0x00);
	outb(0x3f8+3, 0x03);
	outb(0x3f8+2, 0xc7);
	outb(0x3f8+4, 0xb);
	serial_initialized=true;
}

void serial_putc(char c)
{
	if(!serial_initialized)
		serial_init();
	while (!(inb(0x3f8+5)&0x20));
	outb(0x3f8, c);
}

void putc(char c)
{
	switch(c) {
		case '\n': {
			io_putc('\r');
			io_putc('\n');
		} break;
		case '\t': {
			for(int i=0;i<8;i++)
				io_putc(' ');
		} break;
		default: {
			io_putc(c);
		} break;
	}
}

void puts(const char* s)
{
	while(*s)putc(*s++);
}

typedef void (*putc_func)(char);

static void format_num(uint64_t num, int base, int is_upper, int padc, int width, putc_func putc)
{
    char buf[32];
    const char* digits=is_upper?"0123456789ABCDEF":"0123456789abcdef";
    int i=0;

    if (num == 0) buf[i++] = '0';
    else while (num) {
        buf[i++]=digits[num % base];
        num/=base;
    }

    while (i<width) {
        putc(padc);
        width--;
    }
    while (i--) putc(buf[i]);
}

static void format_signed(int64_t val, int base, int is_upper, int padc, int width, putc_func putc)
{
    if (val<0) {
        putc('-');
        val=-val;
    }
    format_num((uint64_t)val, base, is_upper, padc, width, putc);
}

void format_str(const char* fmt, va_list args, putc_func putc)
{
    while (*fmt) {
        if (*fmt=='%') {
            fmt++;
            int padc=' ', width=0;
            if (*fmt=='0') padc='0', fmt++;
            while (*fmt>='0' && *fmt <= '9')
                width=width * 10 + (*fmt++ - '0');

            int is_long=0, is_longlong=0, is_short=0, is_char=0;
            if (*fmt == 'l') {
                fmt++;
                if (*fmt=='l') fmt++, is_longlong=1;
                else is_long=1;
            } else if (*fmt=='h') {
                fmt++;
                if (*fmt=='h') fmt++, is_char=1;
                else is_short=1;
            }

            switch (*fmt) {
                case 'd':
                case 'i': {
                    int64_t val;
                    if (is_longlong) val=va_arg(args, int64_t);
                    else if (is_long) val=va_arg(args, long);
                    else val=va_arg(args, int);
                    if (is_short) val=(short)val;
                    if (is_char) val=(char)val;
                    format_signed(val, 10, 0, padc, width, putc);
                    break;
                }
                case 'u': {
                    uint64_t val;
                    if (is_longlong) val=va_arg(args, uint64_t);
                    else if (is_long) val=va_arg(args, unsigned long);
                    else val=va_arg(args, unsigned int);
                    if (is_short) val=(unsigned short)val;
                    if (is_char) val=(unsigned char)val;
                    format_num(val, 10, 0, padc, width, putc);
                    break;
                }
                case 'x':
                case 'X': {
                    uint64_t val;
                    int up=(*fmt=='X');
                    if (is_longlong) val=va_arg(args, uint64_t);
                    else if (is_long) val=va_arg(args, unsigned long);
                    else val=va_arg(args, unsigned int);
                    if (is_short) val=(unsigned short)val;
                    if (is_char) val=(unsigned char)val;
                    format_num(val, 16, up, padc, width, putc);
                    break;
                }
                case 'p': {
                	void *ptr=va_arg(args, void*);
                	format_num((uint64_t)(uintptr_t)ptr, 16, 0, '0', sizeof(void*), putc);
                	break;
                }
                case 'c': {
                    char c=(char)va_arg(args, int);
                    putc(c);
                    break;
                }
                case 's': {
                    const char* s=va_arg(args, const char*);
                    while (*s) putc(*s++);
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
}

void printf(const char* fmt, ...)
{
	va_list args;
	va_start(args,fmt);
	format_str(fmt, args, putc);
	va_end(args);
}


void Sprintf(const char* fmt, ...)
{
	va_list args;
	va_start(args,fmt);
	format_str(fmt, args, serial_putc);
	va_end(args);
}

char *strcpy(char *dest, const char *src) 
{
    char *ptr=dest;
    while ((*ptr++=*src++));
    return dest;
}


void *memset(void *dest, int val, size_t len) 
{
    uint8_t *ptr=(uint8_t*)dest;
    uint64_t val64=(uint8_t)val;
    val64|=val64<<8;
    val64|=val64<<16;
    val64|=val64<<32;
    
    while (((uintptr_t)ptr&7)&&len) {
        *ptr++=(uint8_t)val;
        len--;
    }
    
    uint64_t *ptr64=(uint64_t*)ptr;
    while (len>=8) {
        *ptr64++=val64;
        len-=8;
    }
    
    ptr=(uint8_t*)ptr64;
    while (len--) {
        *ptr++=(uint8_t)val;
    }
    return dest;
}

int memcmp(const void *a, const void *b, size_t n) 
{
    const uint8_t *p1=(const uint8_t*)a;
    const uint8_t *p2=(const uint8_t*)b;
    for (size_t i=0;i<n;i++) {
        if (p1[i]!=p2[i])
            return p1[i]-p2[i];
    }
    return 0;
}

void *memcpy(void *dest, const void *src, size_t n) 
{
    char *d=(char*)dest;
    const char *s=(const char*)src;
    while(n--) *d++=*s++;
    return dest;
}

char *strncpy(char *dest, const char *src, size_t n) 
{
    size_t i=0;
    while (i<n&&src[i]) {
        dest[i]=src[i];
        i++;
    }
    while (i<n) {
        dest[i++]='\0';
    }
    return dest;
}

int strncmp(const char *s1, const char *s2, size_t n) 
{
    for (size_t i=0;i<n;i++) {
        if (s1[i]!=s2[i]||!s1[i]||!s2[i])
            return (uint8_t)s1[i]-(uint8_t)s2[i];
    }
    return 0;
}

char *strchr(const char *s, int c) 
{
    while (*s) {
        if (*s==(char)c)
            return (char*)s;
        s++;
    }
    return NULL;
}


char *strtok(char *str, const char *delim) 
{
    static char *last=NULL;
    if (str)
    	last=str;
    else if (!last) 
    	return NULL;

    while (*last&&strchr(delim, *last)) 
    	last++;
    if (!*last) 
    	return NULL;

    char *start=last;

    while (*last&&!strchr(delim, *last)) 
    	last++;

    if (*last) {
        *last='\0';
        last++;
    } else {
        last=NULL;
    }

    return start;
}

size_t strlen(const char *s) 
{
    size_t len=0;
    while (*s++) 
    	len++;
    return len;
}

/* Runtime */
uint64_t __udivdi3(uint64_t n, uint64_t d)
{
	if(d==0) {
		return 0xffffffffffffffffULL;
	}
	uint64_t q=0,r=0;
	for(int i=63;i>=0;i--) {
		r<<=1;
		r|=(n>>i)&1;
		if(r>=d) r-=d,q|=(1ULL<<i);
	}
	return q;
}

// uint64_t % uint64_t
uint64_t __umoddi3(uint64_t n, uint64_t d)
{
	if(d==0) {
		return 0xffffffffffffffffULL;
	}
	uint64_t r=0;
	for(int i=63;i>=0;i--) {
		r<<=1;
		r|=(n>>i)&1;
		if(r>=d) r-=d;
	}
	return r;
}
