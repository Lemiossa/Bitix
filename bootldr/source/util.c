/************************************
 * util.c                           *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "vga.h"
#include "real_mode.h"

uint16_t cursor_x = 0, cursor_y = 0;
uint8_t current_attributes = 0x07;

/* Imprime um caractere na tela */
void putc(char c)
{
#ifdef DEBUG
	Regs r = {0};
	r.b.ah = 0x01;
	r.b.al = c;
	r.w.dx = 0;
	intx(0x14, &r);
#endif /* DEBUG */

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

	if (cursor_x >= vga_bottom_right_corner_x) {
		cursor_y++;
		cursor_x = vga_top_left_corner_x;
	}

	if (cursor_y >= vga_bottom_right_corner_y) {
		vga_scroll();
		cursor_x = vga_top_left_corner_x;
		cursor_y = vga_bottom_right_corner_y - 1;
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

/* Imprime uma string formatada */
int printf(const char *fmt, ...)
{
	va_list args;
	char buffer[256];
	int count = 0;
	va_start(args, fmt);
	count = vsnprintf(buffer, sizeof(buffer), fmt, args);
	puts(buffer);
	va_end(args);
	return count;
}

/* Converte um caractere para maiusculo */
char to_upper(char c)
{
	if (c >= 'a' && c <= 'z')
		return c - ('a' - 'A');
	return c;
}

/* Converte um caractere para minusculo */
char to_lower(char c)
{
	if (c >= 'A' && c <= 'Z')
		return c + ('a' - 'A');
	return c;
}

/* Converte uma string para maiusculo */
void str_upper(char *s)
{
	while (*s) {
		*s = to_upper(*s);
		s++;
	}
}

/* Converte uma string para minusculo */
void str_lower(char *s)
{
	while (*s) {
		*s = to_lower(*s);
		s++;
	}
}

/* Separa uma path em **pathparts */
/* Retorna o número de partes de uma path */
int get_path_parts(char *path, char **parts, int max)
{
	if (max == 0 || !path)
		return 0;

	int count = 0;
	char *part = strtok(path, "/\\");
	parts[count++] =  part;

	while (part != NULL && count < max) {
		part = strtok(NULL, "/\\");
		if (part)
			parts[count++] =  part;
	}

	return count;
}

/* Espera N microsegundos usando o BIOS */
void wait_us(uint32_t n)
{
	Regs r = {0};
	r.b.ah = 0x86;
	r.w.cx = (n >> 16) & 0xFFFF;
	r.w.dx = n & 0xFFFF;
	intx(0x15, &r);
}

/* Espera N milisegundos usando o wait_us */
void wait_ms(uint32_t n)
{
	wait_us(n * 1000);
}

/* Espera N segundos usando o wait_ms */
void wait(uint32_t n)
{
	wait_ms(n * 1000);
}

/* Retorna 1 se tiver tecla para ler usando o BIOS */
int kbhit(void)
{
	Regs r = {0};
	r.b.ah = 0x01;
	intx(0x16, &r);
	return !(r.w.flags & FLAG_ZF);
}

/* Pega o codigo ASCII de uma tecla pressionada usando o BIOS */
unsigned char kbgetchar(void)
{
	Regs r = {0};
	r.b.ah = 0x00;
	intx(0x16, &r);
	return r.b.al;
}

/* Pega o scancode de uma tecla pressionada usando o BIOS */
unsigned char kbgetsc(void)
{
	Regs r = {0};
	r.b.ah = 0x00;
	intx(0x16, &r);
	return r.b.ah;
}
