/************************************
 * stdio.c                          *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>

/* Formata uma string com a saída de, no maximo N-1 bytes e coloca 0 em N */
int vsnprintf(char *s, int n, const char *fmt, va_list args)
{
	char *digits = "0123456789abcdef";
	int count = 0;

	while (*fmt && count < (n - 1)) {
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
				count++;
				continue;
			} else if (*fmt == 's') {
				char *str = va_arg(args, char*);
				int buf_len = 0;

				if (!str) {
					str = "(null)";
					buf_len = 6;
				}

				char *str_start = str;

				while (*str) {
					buf_len++;
					str++;
				}
				str = str_start;

				if (!neg_pad) {
					int i = 0;
					for (i = 0; i < pad - buf_len && count < (n - 1); i++) {
						*s++ = ' ';
						count++;
					}
				}

				while (*str && count < (n - 1)) {
					*s++ = *str++;
					count++;
				}

				if (neg_pad) {
					int i = 0;
					for (i = 0; i < pad - buf_len && count < (n - 1); i++) {
						*s++ = ' ';
						count++;
					}
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
				if (count < (n - 1)) {
					*s++ = '-';
					count++;
				}
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
				for (i = 0; i < pad - buf_len && count < (n - 1); i++) {
					*s++ = c;
					count++;
				}
			}

			while (buf_idx-- && count < (n - 1)) {
				*s++ = buf[buf_idx];
				count++;
			}

			if (neg_pad) {
				int i = 0;
				for (i = 0; i < pad - buf_len && count < (n - 1); i++) {
					*s++ = ' ';
					count++;
				}
			}
		} else {
			if (count < (n - 1))
				*s++ = *fmt++;
		}
	}

	*s = 0;
	return  count;
}

/* Formata uma string */
int vsprintf(char *s, const char *fmt, va_list args)
{
	return vsnprintf(s, SIZE_MAX, fmt, args);
}

/* Coloca uma string formatada em um buffer com no maximo n bytes */
int snprintf(char *s, int n, const char *fmt, ...)
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

/* Pula "\r\t\n " */
static void skip_whitespaces(const char **s)
{
	while (**s && strchr("\r\t\n ", **s)) {
		(*s)++;
	}
}

/* Pega um formato em fmt, tenta achar em s e vai colocando nos ponteiros passados */
/* Retorna o número de coisas que encontrou */
int sscanf(const char *s, const char *fmt, ...)
{
	va_list args;
	int count = 0;
	va_start(args, fmt);

	while (*fmt) {
		if (*fmt == '%') {
			fmt++;

			if (*fmt == 'd') {
				int *out = va_arg(args, int *);
				int sign = 1;
				int value = 0;
				int found = 0;

				if (out) {
					skip_whitespaces(&s);

					if (*s == '-') {
						sign = -1;
						s++;
					}

					while (*s >= '0' && *s <= '9') {
						value = value * 10 + (*s - '0');
						s++;
						found = 1;
					}

					*out = value * sign;
				}

				if (found)
					count++;
			} else if (*fmt == 's') {
				char *out = va_arg(args, char *);
				if (out) {
					char *start = out;
					skip_whitespaces(&s);

					while (*s && !strchr("\r\t\n ", *s)) {
						*out++ = *s++;
					}

					*out = 0;
					if ((uint32_t)(out - start) > 0)
						count++;
				}
			} else if (*fmt == 'c') {
				char *out = va_arg(args, char *);
				if (out) {
					if (*s) {
						*out = *s++;
						count++;
					}
				}
			}
		} else {
			if (*fmt == *s)
				s++;
		}

		fmt++;
	}

	va_end(args);
	return count;
}
