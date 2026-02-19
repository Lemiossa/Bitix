/************************************
 * stdio.c                          *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#include <stdint.h>
#include <stdarg.h>
#include "stdio.h"
#include "util.h"
#include "vga.h"

uint16_t cursor_x = 0, cursor_y = 0;
uint8_t current_attributes = 0x07;

/* Imprime um caractere na tela */
void putc(char c)
{
	if (c == '\n') {
		cursor_y++;
	} else if (c == '\r') {
		cursor_x = 0;
	} else if (c == '\t') {
		putc(' ');
		putc(' ');
	} else {
		vga_put_char(cursor_x++, cursor_y, c, current_attributes);
	}

	if (cursor_x >= VGA_WIDTH) {
		cursor_y++;
		cursor_x = 0;
	}

	if (cursor_y >= VGA_HEIGHT) {
		vga_scroll();
		cursor_x = 0;
		cursor_y = VGA_HEIGHT-1;
	}

	vga_set_cursor(cursor_x, cursor_y);
}

/* Imprime uma string na tela */
void puts(const char *s)
{
	while (*s) {
		putc(*s++);
	}
}

/* Formata uma string */
int vsprintf(char *out, const char *fmt, va_list args)
{
	char *digits = "0123456789abcdef";
	char *start = out;

	while (*fmt) {
		if (*fmt == '%') {
			int base = 0;
			int size = 0;
			int sign = 0;
			int upper = 0;
			int pad = 0;
			int neg_pad = 0;
			int zero_pad = 0;
			uint32_t num = 0;
			fmt++;

			if (*fmt == '-') {
				fmt++;
				neg_pad = 1;
			}

			if (*fmt == '0') {
				fmt++;
				zero_pad = 1;
			}

			while (*fmt >= '0' && *fmt <= '9') {
				pad = pad * 10 + (*fmt - '0');
				fmt++;
			}

			if (*fmt == 'h') {
				size = 2;
				fmt++;
				if (*fmt == 'h') {
					size = 1;
					fmt++;
				}
			} else if (*fmt == 'l') {
				size = 4;
				fmt++;
			}

			if (*fmt == 'd') {
				base = 10;
				sign = 1;
			} else if (*fmt == 'u') {
				base = 10;
			} else if (*fmt == 'o') {
				base = 8;
			} else if (*fmt == 'x') {
				base = 16;
			} else if (*fmt == 'X') {
				base = 16;
				upper = 1;
			} else if (*fmt == 'b') {
				base = 2;
			} else if (*fmt == 'c') {
				*out++ = (char)va_arg(args, int);
				fmt++;
				continue;
			} else if (*fmt == 's') {
				char *s = va_arg(args, char*);
				int buf_len = 0;

				if (!s) {
					s = "(null)";
					buf_len = 6;
				}

				char *start = s;

				while (*s) {
					buf_len++;
					s++;
				}
				s = start;

				if (!neg_pad) {
					int i = 0;
					for (i = 0; i < pad - buf_len; i++)
						*out++ = ' ';
				}

				while (*s) {
					*out++ = *s++;
				}

				if (neg_pad) {
					int i = 0;
					for (i = 0; i < pad - buf_len; i++)
						*out++ = ' ';
				}

				fmt++;
				continue;
			}
			fmt++;

			if (sign) {
				int32_t val = va_arg(args, int32_t);

				if (size == 1)
					val = (int8_t)val & 0xFF;
				else if (size == 2)
					val = (int16_t)val & 0xFFFF;

				if (val < 0) {
					val = -val;
				} else {
					sign = 0;
				}

				num = val;
			} else {
				uint32_t val = va_arg(args, uint32_t);

				if (size == 1)
					val = (uint8_t)val & 0xFF;
				else if (size == 2)
					val = (uint16_t)val & 0xFFFF;

				num = val;
			}

			if (sign) {
				*out++ = '-';
			}

			char buf[32];
			int buf_idx = 0;
			int buf_len = 0;
			if (num == 0) {
				buf[buf_idx++] = '0';
			} else {
				while (num > 0) {
					if (upper)
						buf[buf_idx++] = to_upper(digits[num % base]);
					else
						buf[buf_idx++] = digits[num % base];

					num /= base;
				}
			}
			buf_len = buf_idx;

			if (!neg_pad) {
				int i = 0;
				char c = zero_pad?'0':' ';
				for (i = 0; i < pad - buf_len; i++) {
					*out++ = c;
				}
			}

			while (buf_idx--)
				*out++ = buf[buf_idx];

			if (neg_pad) {
				int i = 0;
				for (i = 0; i < pad - buf_len; i++)
					*out++ = ' ';
			}
		} else {
			*out++ = *fmt++;
		}
	}

	*out = 0;
	return (int)(out - start);
}

/* Imprime uma string formatada */
int printf(const char *fmt, ...)
{
	va_list args;
	char buffer[256];
	int count = 0;
	va_start(args, fmt);
	count = vsprintf(buffer, fmt, args);
	puts(buffer);
	va_end(args);
	return count;
}
