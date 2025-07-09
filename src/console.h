#ifndef CONSOLE_H
#define CONSOLE_H

#include "types.h"

typedef struct {
	uchar chr;
	uchar attr;
} ct;

#define KEY_ESC 0x100 + 0x01
#define KEY_UP 0x100 + 0x48
#define KEY_DOWN 0x100 + 0x50
#define KEY_LEFT 0x100 + 0x4C
#define KEY_RIGHT 0x100 + 0x4D

#define CTRL_CHAR(c) ((c) & 0x1f)
#define MAX_LINE 256

extern uchar cursor_x, cursor_y;

void putc (uchar c);
void puts (uchar *str);
void setcursor (uchar x, uchar y);
void clear_screen (uchar attr);
void kputsf (uchar *format, ...);
void putxy (uchar x, uchar y, uchar c, uchar attr);

#endif /* CONSOLE_H */
