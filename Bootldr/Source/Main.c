/************************************
 * Main.c                           *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#include "Types.h"
#include "Vga.h"

uint16_t cursor_x = 0, cursor_y = 0;
uint8_t current_attributes = 0x07;

/* Imprime um caractere na tela */
void putc(char c)
{
	if (c == '\n') {
		cursor_y++;
	} else if (c == '\r') {
		cursor_x = 0;
	} else {
		vga_put_char(cursor_x++, cursor_y, c, current_attributes);
	}

	if (cursor_x >= VGA_WIDTH) {
		cursor_y++;
		cursor_x = 0;
	}

	if (cursor_y >= VGA_HEIGHT) {
		vga_scroll();
	}

	vga_set_cursor(cursor_x, cursor_y);
}

/* Imprime uma string na tela */
void puts(char *s)
{
	while (*s) {
		putc(*s++);
	}
}

int main(void)
{
	vga_clear();
	puts("Hello World\r\n");
	while (1);
}
