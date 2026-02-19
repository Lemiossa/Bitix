/************************************
 * Main.c                           *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#include <stdint.h>
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

typedef union Regs {
	struct {
		uint8_t al, ah, __al, __ah;
		uint8_t bl, bh, __bl, __bh;
		uint8_t cl, ch, __cl, __ch;
		uint8_t dl, dh, __dl, __dh;
	} b;

	struct {
		uint16_t ax, __ax;
		uint16_t bx, __bx;
		uint16_t cx, __cx;
		uint16_t dx, __dx;
		uint16_t bp, __bp;
		uint16_t si, __si;
		uint16_t di, __di;
		uint16_t ds;
		uint16_t es;
	} w;

	struct {
		uint32_t eax;
		uint32_t ebx;
		uint32_t ecx;
		uint32_t edx;
		uint32_t ebp;
		uint32_t esi;
		uint32_t edi;
	} d;
} Regs;

void int16(uint8_t intnum, Regs *r);

int main(void)
{
	vga_clear(0x07);
	puts("Hello World\r\n");

	Regs r = {0};
	r.b.ah = 0x0E;
	r.b.al = 'A';
	int16(0x10, &r);

	while (1);
}
