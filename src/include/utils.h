#ifndef UTILS_H
#define UTILS_H

#include <types.h>

/* VGA */
void setcolor(uchar bg, uchar fg);
void vga_enable_blink();
void vga_disable_blink();

/* Text Mode */
void puts(uchar *str);
void putsp(uchar *s, int width, uchar pad_char, bool neg);
void kputsf(uchar *format, ...);

/* String functions */
uint streq(uchar *a, uchar *b);
uint strneq(uchar *a, uchar *b, uint n);
char *numtostr(ulong val, uchar *out, ushort base, bool neg);

#endif /* UTILS_H */
