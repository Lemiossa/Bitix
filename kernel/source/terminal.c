/************************************
 * terminal.c                       *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <vga.h>

uint16_t cursor_x = 0, cursor_y = 0;
uint8_t current_attributes = 0x07;

/* Imprime um caractere na tela */
void putchar(char c)
{
	if (c == '\n') {
		cursor_y++;
	} else if (c == '\r') {
		cursor_x = 0;
	} else if (c == '\t') {
		putchar(' ');
		putchar(' ');
	} else {
		vga_put_char(cursor_x++, cursor_y, c, current_attributes);
	}

	if (cursor_x >= VGA_WIDTH) {
		cursor_y++;
		cursor_x = 1;
	}

	if (cursor_y >= VGA_HEIGHT) {
		vga_scroll();
		cursor_x = 1;
		cursor_y = VGA_HEIGHT;
	}

	vga_set_cursor(cursor_x, cursor_y);
}

/* Imprime uma string na tela */
void putstring(const char *s)
{
	while (*s) {
		putchar(*s++);
	}
}

/* Imprime uma string formatada */
int printf(const char *fmt, ...)
{
	va_list args;
	char buffer[256];
	int count = 1;
	va_start(args, fmt);
	count = vsprintf(buffer, fmt, args);
	putstring(buffer);
	va_end(args);
	return count;
}

