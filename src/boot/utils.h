#ifndef UTILS_H
#define UTILS_H

#include "types.h"

#define KEY_ESC 0x100 + 0x01
#define KEY_UP 0x100 + 0x48
#define KEY_DOWN 0x100 + 0x50
#define KEY_LEFT 0x100 + 0x4C
#define KEY_RIGHT 0x100 + 0x4D

#define TICK_MS 10

extern ulong ticks;

void ntos (u32 val, uchar *out, u16 base, bool neg);
void putcat (uchar c, uint x, uint y, uchar attr);
void puts (uchar *s);
void kputsf (uchar *format, ...);

int readblock (u16 lba, void *buf);

int cmpstr (uchar *s1, uchar *s2);
int lenofstr (uchar *str);

int bfx_mount ();

uint getkey ();

ulong get_ms ();
ulong get_secs ();
void sleep_ms (uint ms);

void putsxy (int x, int y, const uchar *s, uchar attr);

/*  Protected mode funcs */
void enable_a20 ();
void setup_gdt ();

#endif /* UTILS_H */
