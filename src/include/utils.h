#ifndef UTILS_H
#define UTILS_H

#include <types.h>

#define KEY_ESC   0x100+0x01
#define KEY_UP    0x100+0x48
#define KEY_DOWN  0x100+0x50
#define KEY_LEFT  0x100+0x4C
#define KEY_RIGHT 0x100+0x4D

/* VGA */
void setcolor(uchar bg, uchar fg);

/* Text Mode */
extern uchar cursor_x, cursor_y;

void putc(uchar c);
void puts(uchar *str);
void putsp(uchar *s, int width, uchar pad_char, bool neg);
void kputsf(uchar *format, ...);
void setcursor(uchar x, uchar y);
void clear_screen(uchar attr);

/* String functions */
uint streq(uchar *a, uchar *b);
uint strneq(uchar *a, uchar *b, uint n);
char *numtostr(ulong val, uchar *out, ushort base, bool neg);
uint strlen(uchar *s);
char *strstr(uchar *haystack, uchar *needle);
char *strchr(uchar *s, int c);

/* Filesystem */
int bfx_mount();
int bfx_readfile(uchar *path, ushort seg, ushort off);

/* Others */
int readblock(u16 lba, void *buf);
void hexdump(void *data, uint size);

#endif /* UTILS_H */
