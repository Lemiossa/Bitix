/**
 * Console.c
 * Created by Matheus Leme Da Silva
 */
#include "console.h"
#include "types.h"
#include "utils.h"
#include "video.h"
#include "x86.h"

uchar cursor_x, cursor_y;

enum { STATE_TEXT, STATE_ESC, STATE_CSI };
static ansi_state = STATE_TEXT;
static uchar params[8];
static int param_count = 0;
static int cur_param = 0;

uchar current_attr = 0x07;

int shift_pressed = 0;
int caps_lock = 0;
int ctrl_pressed = 0;

/**
 * Change the chacters attributes
 */
void
setcolor (uchar bg, uchar fg)
{
	current_attr = ((bg & 0x07) << 4) | (fg & 0x0f);
}

/**
 * Scroll the screen
 */
void
scroll ()
{
	ushort src_off = screen_width * 2;
	ushort size = (screen_height - 1) * screen_width * 2;
	ushort y, x;
	lmemmove (0xb800, 0, 0xb800, src_off, size);
	y = screen_height - 1;
	for (x = 0; x < screen_width; x++) {
		ushort off = (y * screen_width + x) * 2;
		lwriteb (0xb800, off, ' ');
		lwriteb (0xb800, off + 1, current_attr);
	}
	if (cursor_y > 0)
		cursor_y--;
}

/**
 * Print char in the screen
 */
static void
io_putc (uchar c)
{
	ushort pos, offset;
	serial_putc (c);
	if (c == '\n') {
		cursor_x = 0;
		cursor_y++;
		if (cursor_y >= screen_height) {
			scroll ();
		}
		setcursor (cursor_x, cursor_y);
	} else if (c == '\r') {
		cursor_x = 0;
	} else if (c == '\b') {
		if (cursor_x > 0)
			cursor_x--;
		else if (cursor_y > 0) {
			cursor_y--;
			cursor_x = screen_width - 1;
		}
		pos = cursor_y * screen_width + cursor_x;
		offset = pos * 2;
		lwriteb (0xb800, offset, ' ');
		lwriteb (0xb800, offset + 1, current_attr);
	} else if (c == '\t') {
		do {
			pos = cursor_y * screen_width + cursor_x;
			offset = pos * 2;
			lwriteb (0xb800, offset, ' ');
			lwriteb (0xb800, offset + 1, current_attr);
			cursor_x++;
			if (cursor_x >= screen_width) {
				cursor_x = 0;
				cursor_y++;
				if (cursor_y >= screen_height)
					scroll ();
			}
		} while (cursor_x % 4 != 0);
	} else {
		pos = cursor_y * screen_width + cursor_x;
		offset = pos * 2;
		lwriteb (0xb800, offset, c);
		lwriteb (0xb800, offset + 1, current_attr);
		cursor_x++;
		if (cursor_x >= screen_width) {
			cursor_x = 0;
			cursor_y++;
			if (cursor_y >= screen_height)
				scroll ();
		}
	}
	setcursor (cursor_x, cursor_y);
}

/**
 * Put char in console
 */
static uchar
ansi_to_vga_color (uchar ansi)
{
	static uchar table[8] = { 0, 4, 2, 6, 1, 5, 3, 7 };
	return (ansi < 8) ? table[ansi] : 7;
}
void
putc (uchar c)
{
	switch (ansi_state) {
	case STATE_TEXT: {
		if (c == 0x1b) {
			ansi_state = STATE_ESC;
		} else {
			io_putc (c);
		}
	} break;
	case STATE_ESC: {
		if (c == '[') {
			int i;
			ansi_state = STATE_CSI;
			param_count = 0;
			cur_param = 0;
			for (i = 0; i < 8; i++)
				params[i] = 0;
		} else {
			ansi_state = STATE_TEXT;
			putc (0x1b);
			putc (c);
		}
	} break;
	case STATE_CSI: {
		if (c >= '0' && c <= '9') {
			cur_param = cur_param * 10 + (c - '0');
		} else if (c == ';') {
			if (param_count < 8)
				params[param_count++] = cur_param;
			cur_param = 0;
		} else if (c == 'm') {
			int i;
			if (param_count < 8)
				params[param_count++] = cur_param;
			for (i = 0; i < param_count; i++) {
				uchar p = params[i];
				if (p == 0) {
					setcolor (0, 7);
					current_attr &= ~0x80;
				} else if (p >= 30 && p <= 37) {
					uchar fg = ansi_to_vga_color (p - 30);
					current_attr = (current_attr & 0xf8) | (fg & 0x0f);
				} else if (p >= 40 && p <= 47) {
					uchar bg = ansi_to_vga_color (p - 40);
					current_attr = (current_attr & 0x8f) | ((bg & 0x07) << 4);
				} else if (p == 5) {
					current_attr |= 0x80;
				} else if (p == 25) {
					current_attr &= ~0x80;
				}
			}
			ansi_state = STATE_TEXT;
		} else {
			ansi_state = STATE_TEXT;
		}
	} break;
	}
}

/**
 * Return 1 if any key as pressed
 */
uchar
kbd_has_data ()
{
	return inportb (0x64) & 1;
}

/**
 * Get keyboard scancode
 */
uchar
kbd_read_scanconde ()
{
	while (!(inportb (0x64) & 1))
		;
	return inportb (0x60);
}

/**
 * Get KEY
 * Extended for ESPECIAL KEYS
 */
uint
getkey ()
{
	uchar code = kbd_read_scanconde ();

	if (code & 0x80) {
		uchar released = code & 0x7f;
		if (released == 0x2a || released == 0x36)
			shift_pressed = 0;
		if (released == 0x1d)
			ctrl_pressed = 0;
		return 0;
	}

	if (code == 0x2a || code == 0x36) {
		shift_pressed = 1;
		return 0;
	}

	if (code == 0x3a) {
		caps_lock ^= 1;
		return 0;
	}

	if (code == 0x1d) {
		ctrl_pressed = 1;
		return 0;
	}

	switch (code) {
	case 0x01:
		return KEY_ESC;
	case 0x02:
		return shift_pressed ? '!' : '1';
	case 0x03:
		return shift_pressed ? '@' : '2';
	case 0x04:
		return shift_pressed ? '#' : '3';
	case 0x05:
		return shift_pressed ? '$' : '4';
	case 0x06:
		return shift_pressed ? '%' : '5';
	case 0x07:
		return shift_pressed ? '^' : '6';
	case 0x08:
		return shift_pressed ? '&' : '7';
	case 0x09:
		return shift_pressed ? '*' : '8';
	case 0x0a:
		return shift_pressed ? '(' : '9';
	case 0x0b:
		return shift_pressed ? ')' : '0';
	case 0x0c:
		return shift_pressed ? '_' : '-';
	case 0x0d:
		return shift_pressed ? '+' : '=';

	case 0x1a:
		return shift_pressed ? '{' : '[';
	case 0x1b:
		return shift_pressed ? '}' : ']';
	case 0x2b:
		return shift_pressed ? '\\' : '|';
	case 0x27:
		return shift_pressed ? ':' : ';';
	case 0x28:
		return shift_pressed ? '"' : '\'';
	case 0x29:
		return shift_pressed ? '~' : '`';
	case 0x33:
		return shift_pressed ? '<' : ',';
	case 0x34:
		return shift_pressed ? '>' : '.';
	case 0x35:
		return shift_pressed ? '?' : '/';

	case 0x10:
		return ctrl_pressed ? CTRL_CHAR ('Q') : (shift_pressed ^ caps_lock) ? 'Q' : 'q';
	case 0x11:
		return ctrl_pressed ? CTRL_CHAR ('W') : (shift_pressed ^ caps_lock) ? 'W' : 'w';
	case 0x12:
		return ctrl_pressed ? CTRL_CHAR ('E') : (shift_pressed ^ caps_lock) ? 'E' : 'e';
	case 0x13:
		return ctrl_pressed ? CTRL_CHAR ('R') : (shift_pressed ^ caps_lock) ? 'R' : 'r';
	case 0x14:
		return ctrl_pressed ? CTRL_CHAR ('T') : (shift_pressed ^ caps_lock) ? 'T' : 't';
	case 0x15:
		return ctrl_pressed ? CTRL_CHAR ('Y') : (shift_pressed ^ caps_lock) ? 'Y' : 'y';
	case 0x16:
		return ctrl_pressed ? CTRL_CHAR ('U') : (shift_pressed ^ caps_lock) ? 'U' : 'u';
	case 0x17:
		return ctrl_pressed ? CTRL_CHAR ('I') : (shift_pressed ^ caps_lock) ? 'I' : 'i';
	case 0x18:
		return ctrl_pressed ? CTRL_CHAR ('O') : (shift_pressed ^ caps_lock) ? 'O' : 'o';
	case 0x19:
		return ctrl_pressed ? CTRL_CHAR ('P') : (shift_pressed ^ caps_lock) ? 'P' : 'p';
	case 0x1e:
		return ctrl_pressed ? CTRL_CHAR ('A') : (shift_pressed ^ caps_lock) ? 'A' : 'a';
	case 0x1f:
		return ctrl_pressed ? CTRL_CHAR ('S') : (shift_pressed ^ caps_lock) ? 'S' : 's';
	case 0x20:
		return ctrl_pressed ? CTRL_CHAR ('D') : (shift_pressed ^ caps_lock) ? 'D' : 'd';
	case 0x21:
		return ctrl_pressed ? CTRL_CHAR ('F') : (shift_pressed ^ caps_lock) ? 'F' : 'f';
	case 0x22:
		return ctrl_pressed ? CTRL_CHAR ('G') : (shift_pressed ^ caps_lock) ? 'G' : 'g';
	case 0x23:
		return ctrl_pressed ? CTRL_CHAR ('H') : (shift_pressed ^ caps_lock) ? 'H' : 'h';
	case 0x24:
		return ctrl_pressed ? CTRL_CHAR ('J') : (shift_pressed ^ caps_lock) ? 'J' : 'j';
	case 0x25:
		return ctrl_pressed ? CTRL_CHAR ('K') : (shift_pressed ^ caps_lock) ? 'K' : 'k';
	case 0x26:
		return ctrl_pressed ? CTRL_CHAR ('L') : (shift_pressed ^ caps_lock) ? 'L' : 'l';
	case 0x2c:
		return ctrl_pressed ? CTRL_CHAR ('Z') : (shift_pressed ^ caps_lock) ? 'Z' : 'z';
	case 0x2d:
		return ctrl_pressed ? CTRL_CHAR ('X') : (shift_pressed ^ caps_lock) ? 'X' : 'x';
	case 0x2e:
		return ctrl_pressed ? CTRL_CHAR ('C') : (shift_pressed ^ caps_lock) ? 'C' : 'c';
	case 0x2f:
		return ctrl_pressed ? CTRL_CHAR ('V') : (shift_pressed ^ caps_lock) ? 'V' : 'v';
	case 0x30:
		return ctrl_pressed ? CTRL_CHAR ('B') : (shift_pressed ^ caps_lock) ? 'B' : 'b';
	case 0x31:
		return ctrl_pressed ? CTRL_CHAR ('N') : (shift_pressed ^ caps_lock) ? 'N' : 'n';
	case 0x32:
		return ctrl_pressed ? CTRL_CHAR ('M') : (shift_pressed ^ caps_lock) ? 'M' : 'm';

	case 0x48:
		return KEY_UP;
	case 0x50:
		return KEY_DOWN;
	case 0x4b:
		return KEY_LEFT;
	case 0x4d:
		return KEY_RIGHT;

	case 0x39:
		return ' ';
	case 0x1c:
		return '\r';
	case 0x0e:
		return '\b';
	case 0x0f:
		return '\t';

	default:
		return 0;
	}
}

/**
 * Read line in buffer
 */
int
readline (uchar *buffer, int max_len)
{
	int pos = 0;
	uint len = 0;
	while (1) {
		uint key = getkey ();
		len = strlen (buffer);
		if (key == 0)
			continue;
		if (key == '\r') {
			putc ('\n');
			buffer[pos] = 0;
			return pos;
		} else if (key == '\b') {
			if (pos > 0) {
				pos--;
				putc ('\b');
				putc (' ');
				putc ('\b');
			}
		} else if (key == KEY_LEFT) {
			if (pos > 0) {
				setcursor (cursor_x - 1, cursor_y);
				pos--;
			}
		} else if (key == KEY_RIGHT) {
			if (pos < len) {
				setcursor (cursor_x + 1, cursor_y);
				pos++;
			}
		} else {
			if (pos < max_len - 1) {
				if (key >= 32 && key <= 126) {
					putc ((uchar)key);
					buffer[pos++] = (uchar)key;
				}
			}
		}
	}
}

/**
 * Set cursor position
 */
void
setcursor (uchar x, uchar y)
{
	ushort pos = y * screen_width + x;
	if (video_mode != 1)
		return;
	outportb (0x3d4, 0x0e);
	outportb (0x3d5, (pos >> 8) & 0xff);
	outportb (0x3d4, 0x0f);
	outportb (0x3d5, pos & 0xff);
	cursor_x = x;
	cursor_y = y;
}

/**
 * Clear screen with attribute
 */
void
clear_screen (uchar attr)
{
	ushort x, y, offset;
	for (y = 0; y < screen_height; y++) {
		for (x = 0; x < screen_width; x++) {
			offset = (y * screen_width + x) * 2;
			lwriteb (0xb800, offset, ' ');
			lwriteb (0xb800, offset + 1, attr);
		}
	}
	cursor_x = 0;
	cursor_y = 0;
	current_attr = attr;
	setcursor (cursor_x, cursor_y);
}

/**
 * Put string with padding
 */
void
putsp (uchar *s, int width, uchar pad_char, bool neg)
{
	int len = 0;
	int pad;
	while (s[len])
		len++;
	pad = (width > 0) ? (width - len) : (-width - len);
	if (pad < 0)
		pad = 0;
	if (!neg) {
		while (pad-- > 0)
			putc (pad_char);
		puts (s);
	} else {
		puts (s);
		while (pad-- > 0)
			putc (pad_char);
	}
}

/**
 * Put string formatted
 */
#define __max_buf_size 32
void
kputsf (uchar *format, ...)
{
	uint *arg = (uint *)(&format + 1);
	while (*format) {
		if (*format == '%') {
			uchar buf[__max_buf_size];
			long val;
			ulong uval;
			long neg;
			bool is_long = false;
			uint base = 10;
			int width = 0;
			bool width_neg = false;
			uchar pad_char = ' ';

			format++;

			if (*format == '{' && format[2] == '}') {
				pad_char = format[1];
				format += 3;
			}

			if (*format == '-') {
				width_neg = true;
				width = -width;
				format++;
			}

			if (*format == '*') {
				width = *((uint *)arg++);
				format++;
			} else {
				while (*format >= '0' && *format <= '9') {
					width = width * 10 + (*format - '0');
					format++;
				}
			}

			if (*format == 'l') {
				val = *((ulong *)arg);
				arg += 2;
				is_long = true;
				format++;
			} else {
				val = *((uint *)arg++);
				is_long = false;
			}
			uval = val;
			neg = ((val & (is_long ? 0x80000000 : 0x8000)) ? true : false);
			if (neg) {
				if (is_long) {
					val = (~val) + 1;
				} else {
					val = (ulong)(-(slong)val) + 1;
				}
			}

			if (*format == 'd' || *format == 'i') {
				base = 10;
			} else if (*format == 'u') {
				base = 10;
				neg = false;
				val = uval;
			} else if (*format == 'x') {
				base = 16;
			} else if (*format == 'b') {
				base = 2;
			} else if (*format == 'o') {
				base = 8;
			} else if (*format == '%') {
				puts ("%");
				format++;
				continue;
			} else if (*format == 's') {
				uchar *str = val;
				if (str) {
					if (width)
						putsp (str, width, ' ', width_neg);
					else
						puts (str);
				}
				format++;
				continue;
			} else {
				format++;
				continue;
			}
			format++;
			numtostr (val, buf, base, neg);
			if (width)
				putsp (buf, width, pad_char, width_neg);
			else
				puts (buf);
		} else {
			putc (*format++);
		}
	}
}

/**
 * Puts string in the screen
 */
void
puts (uchar *str)
{
	while (*str)
		putc (*str++);
}

/**
 * Put char at position
 */
void
putxy (uchar x, uchar y, uchar c, uchar attr)
{
	ushort offset;
	if (video_mode != 1)
		return;

	if (x >= screen_width || y >= screen_height)
		return;
	offset = (y * screen_width + x) * 2;
	lwriteb (0xb800, offset, c);
	lwriteb (0xb800, offset + 1, attr);
}
