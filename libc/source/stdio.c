/************************************
 * stdio.c                          *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>

/* Formata uma string com a saída de, no maximo N bytes */
int vsnprintf(char *s, size_t n, const char *fmt, va_list args)
{
	char *digits = "0123456789abcdef";
	char *start = s;

	while (*fmt && (size_t)(s - start) < n) {
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
				if (*fmt == 'l') {
					size = 8;
					fmt++;
				}
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
				*s++ = (char)va_arg(args, int);
				fmt++;
				continue;
			} else if (*fmt == 's') {
				char *str = va_arg(args, char*);
				int buf_len = 0;

				if (!str) {
					str = "(null)";
					buf_len = 6;
				}

				char *start = str;

				while (*str) {
					buf_len++;
					str++;
				}
				str = start;

				if (!neg_pad) {
					int i = 0;
					for (i = 0; i < pad - buf_len && (size_t)(s - start) < n; i++)
						*s++ = ' ';
				}

				while (*s && (size_t)(s - start) < n) {
					*s++ = *str++;
				}

				if (neg_pad) {
					int i = 0;
					for (i = 0; i < pad - buf_len && (size_t)(s - start) < n; i++)
						*s++ = ' ';
				}

				fmt++;
				continue;
			}
			fmt++;

			if (sign) {
				int32_t val = (int32_t)va_arg(args, int32_t);

				if (size == 1)
					val = (int8_t)val;
				else if (size == 2)
					val = (int16_t)val;

				if (val < 0) {
					val = -val;
				} else {
					sign = 0;
				}

				num = val;
			} else {
				uint32_t val = va_arg(args, uint32_t);

				if (size == 1)
					val = (uint8_t)val;
				else if (size == 2)
					val = (uint16_t)val;

				num = val;
			}

			if (sign) {
				if ((size_t)(s - start) < n)
					*s++ = '-';
			}

			char buf[32] = {0};
			int buf_idx = 0;
			int buf_len = 0;
			if (num == 0) {
				buf[buf_idx++] = '0';
			} else {
				while (num > 0) {
					char c = digits[num % base];
					if (upper)
						buf[buf_idx++] = c >= 'a' && c <= 'z' ? c - ('a' - 'A') : c;
					else
						buf[buf_idx++] = c;

					num /= base;
				}
			}
			buf_len = buf_idx;

			if (!neg_pad) {
				int i = 0;
				char c = zero_pad?'0':' ';
				for (i = 0; i < pad - buf_len && (size_t)(s - start) < n; i++) {
					*s++ = c;
				}
			}

			while (buf_idx-- && (size_t)(s - start) < n)
				*s++ = buf[buf_idx];

			if (neg_pad) {
				int i = 0;
				for (i = 0; i < pad - buf_len && (size_t)(s - start) < n; i++)
					*s++ = ' ';
			}
		} else {
			if ((size_t)(s - start) < n)
				*s++ = *fmt++;
		}
	}

	*s = 0;
	return (int)(s - start);
}

/* Formata uma string */
int vsprintf(char *s, const char *fmt, va_list args)
{
	return vsnprintf(s, SIZE_MAX, fmt, args);
}

/* Coloca uma string formatada em um buffer com no maximo n bytes */
int snprintf(char *s, size_t n, const char *fmt, ...)
{
	va_list args;
	int count = 0;
	va_start(args, fmt);
	count = vsnprintf(s, n, fmt, args);
	va_end(args);
	return count;
}

/* Coloca uma string formatada em um buffer */
int sprintf(char *s, const char *fmt, ...)
{
	va_list args;
	int count = 0;
	va_start(args, fmt);
	count = snprintf(s, SIZE_MAX, fmt, args);
	va_end(args);
	return count;
}
