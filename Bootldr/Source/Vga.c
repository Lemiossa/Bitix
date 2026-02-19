/************************************
 * Vga.c                            *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#include "Types.h"
#include "Io.h"
#include "Vga.h"

uint16_t *vga = (uint16_t *)0xB8000;

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
	for (int y = 1; y < VGA_HEIGHT; y++) {
		for (int x = 0; x < VGA_WIDTH; x++) {
			uint16_t cell = vga_get_char(x, y);
			char c = cell & 0xFF;
			uint8_t attributes = (cell >> 8) & 0xFF;
			vga_put_char(x, y - 1, c, attributes);
		}
	}

	for (int x = 0; x < VGA_WIDTH; x++) {
		uint16_t cell = vga_get_char(x, VGA_HEIGHT-1);
		uint8_t attributes = (cell >> 8) & 0xFF;
		vga_put_char(x, VGA_HEIGHT-1, ' ', attributes);
	}
}

/* Limpa a tela no modo de texto VGA */
void vga_clear(void)
{
	for (uint16_t y = 0; y < VGA_HEIGHT; y++) {
		for (uint16_t x = 0; x < VGA_WIDTH; x++) {
			vga_put_char(x, y, ' ', 0x07);
		}
	}
}

