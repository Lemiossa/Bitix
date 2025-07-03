#ifndef UTILS_H
#define UTILS_H

#include <types.h>

/* VGA */
void setcolor(uchar bg, uchar fg);
void vga_enable_blink();
void vga_disable_blink();
void io_set_video_mode(uchar mode);

/* Text Mode */
extern uchar cursor_x, cursor_y;

void putc(uchar c);
void puts(uchar *str);
void putsp(uchar *s, int width, uchar pad_char, bool neg);
void kputsf(uchar *format, ...);
void setcursor(uchar x, uchar y);
void clear_screen(uchar attr);
void set80x50mode();

/* String functions */
uint streq(uchar *a, uchar *b);
uint strneq(uchar *a, uchar *b, uint n);
char *numtostr(ulong val, uchar *out, ushort base, bool neg);
uint strlen(uchar *s);

/* Filesystem */
int bfx_mount();
int bfx_readfile(uchar *path, ushort seg, ushort off);

/* Others */
void pic_remap();
int readblock(u16 lba, void *buf);
void hexdump(void *data, uint size);

#endif /* UTILS_H */
