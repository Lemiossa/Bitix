#ifndef LIBK_H
#define LIBK_H

#include "types.h"
#include "hardx86.h"

#define KEY_UP    0x100+0x48
#define KEY_DOWN  0x100+0x50
#define KEY_LEFT  0x100+0x4C
#define KEY_RIGHT 0x100+0x4D

#define TICK_MS 10

#ifndef __80x50_mode__
#define CONSOLE_HEIGHT 25
#else
#define CONSOLE_HEIGHT  50
#endif
#define CONSOLE_WIDTH   80

extern ulong ticks;

void ntos(u32 val, uchar *out, u16 base, bool neg);
int ston(uchar *s);

void putc(uchar c);
void putcat(uchar c, uint x, uint y, uchar attr);
void puts(uchar *s);
void kputsf(uchar *format, ...);
uint getkey();
void clear_screen();

ulong get_ms();
ulong get_secs();
void sleep_ms(uint ms);

int cmpstr(uchar *s1, uchar *s2);
int lenofstr(uchar *str);
void *memcopy(void *dest, void *src, uint len);
void *lmemcopy(void *dest, u16 src_seg, u16 src_off, uint len);
bool cmpmem(uchar *a, uchar *b, uint len);
void setmem(uchar *dst, uchar val, uint len);
void readline(uchar *buf, uint max);

static ushort bswap16(ushort x) { return (x>>8)|(x<<8); }

void dialog_box(uchar *title, uchar *message);

#endif /* LIBK_H */
