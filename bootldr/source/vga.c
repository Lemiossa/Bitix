/************************************
 * vga.c                            *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#include <stdint.h>
#include <stdio.h>
#include "io.h"
#include "vga.h"
#include "real_mode.h"
#include "util.h"

uint16_t *vga = (uint16_t *)0xB8000;
uint16_t vga_top_left_corner_x = 0, vga_top_left_corner_y = 0;
uint16_t vga_bottom_right_corner_x = VGA_WIDTH , vga_bottom_right_corner_y = VGA_HEIGHT - 1;

/* Muda a posição do cursor no modo de texto VGA */
void vga_set_cursor(uint16_t x, uint16_t y)
{
	uint16_t pos = y * VGA_WIDTH + x;

	outb(0x3D4, 0x0F);
	outb(0x3D5, (uint8_t)(pos & 0xFF));
	outb(0x3D4, 0x0E);
	outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

/* Desenha um caractere diretamente no modo de texto VGA */
void vga_put_char(uint16_t x, uint16_t y, char c, uint8_t attributes)
{
	if (x >= VGA_WIDTH || y >= VGA_HEIGHT)
		return;

	uint8_t uc = (uint8_t)c;
	uint16_t pos = y * VGA_WIDTH + x;
	vga[pos] = uc | (attributes << 8);
}

/* Desenha uma string diretamente no modo de texto VGA */
void vga_put_string(uint16_t x, uint16_t y, char *s, uint8_t attributes)
{
	if (x >= VGA_WIDTH || y >= VGA_HEIGHT)
		return;

	while (*s) {
		vga_put_char(x++, y, *s++, attributes);

		if (x >= VGA_WIDTH) {
			x = 0;
			y++;
		}

		if (y >= VGA_HEIGHT)
			vga_scroll();
	}
}

/* Retorna um caractere de uma posição específica do modo de texto VGA */
uint16_t vga_get_char(uint16_t x, uint16_t y)
{
	if (x >= VGA_WIDTH || y >= VGA_HEIGHT)
		return 0;

	uint16_t pos = y * VGA_WIDTH + x;
	return vga[pos];
}

/* Faz o scroll de uma linha no modo de texto VGA */
void vga_scroll(void)
{
	for (int y = vga_top_left_corner_y; y < vga_bottom_right_corner_y; y++) {
		for (int x = vga_top_left_corner_x; x < vga_bottom_right_corner_x; x++) {
			uint16_t cell = vga_get_char(x, y);
			char c = cell & 0xFF;
			uint8_t attributes = (cell >> 8) & 0xFF;
			vga_put_char(x, y - 1, c, attributes);
		}
	}

	for (int x = vga_top_left_corner_x; x < vga_bottom_right_corner_x; x++) {
		uint16_t cell = vga_get_char(x, vga_bottom_right_corner_y);
		uint8_t attributes = (cell >> 8) & 0xFF;
		vga_put_char(x, vga_bottom_right_corner_y, ' ', attributes);
	}
}

/* Limpa a tela no modo de texto VGA */
void vga_clear(uint8_t attributes)
{
	for (uint16_t y = vga_top_left_corner_y; y < vga_bottom_right_corner_y; y++) {
		for (uint16_t x = vga_top_left_corner_x; x < vga_bottom_right_corner_x; x++) {
			vga_put_char(x, y, ' ', attributes);
		}
	}
}

/* Pega a fonte VGA */
/* Retorna o ponteiro e coloca o número de bytes por caractere em height */
uint8_t *vga_get_font(uint8_t pointer, uint16_t *height)
{
	Regs r = {0};
	r.w.ax = 0x1130;
	r.b.bh = pointer;
	intx(0x10, &r);

	if (height)
		*height = r.w.cx;

#ifdef DEBUG
	printf("r.w.cx = %04X\r\n", r.w.cx);
#endif /* DEBUG */

	uint8_t *font = (uint8_t *)MK_PTR(r.w.es, r.w.bp);
#ifdef DEBUG
	printf("vga_font = %08X\r\n", (uint32_t)font);
#endif /* DEBUG */

	return font;
}
