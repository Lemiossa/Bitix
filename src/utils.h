#ifndef UTILS_H
#define UTILS_H

#include "types.h"

/* String functions */
uint streq (uchar *a, uchar *b);
uint strneq (uchar *a, uchar *b, uint n);
char *numtostr (ulong val, uchar *out, ushort base, bool neg);
uint strlen (uchar *s);
char *strstr (uchar *haystack, uchar *needle);
char *strchr (uchar *s, int c);
void strcpy (uchar *dst, uchar *src);
char *strrchr (uchar *s, int c);
void settext (uchar *regs, int cols, int rows, int char_height);
void strcat (uchar *dst, uchar *src);
void *memcpy (void *dst, void *src, uint size);
void memset (void *ptr, uchar val, uint size);
void lmemmove (ushort dst_seg, ushort dst_off, ushort src_seg, ushort src_off, ushort size);
void hexdump (void *data, uint size);
ulong getms ();
void sleep (ushort ms);
void int86 (uchar intnum, regs16_t *in, regs16_t *out);
int exec (uchar *path);
void lmemcpy (void *buf, ushort seg, ushort off, uint size);
void lrmemcpy (ushort seg, ushort off, void *buf, uint size);

#endif /* UTILS_H */
