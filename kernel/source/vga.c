/************************************
 * vga.c                            *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#include <stdint.h>
#include <io.h>
#include <vga.h>

uint16_t *vga = (uint16_t *)0xB8000;

/* Muda a posição do cursor no modo de texto VGA */
void vga_set_cursor(int x, int y)
{
	uint16_t pos = y * VGA_WIDTH + x;

	outb(0x3D4, 0x0F);
	outb(0x3D5, (uint8_t)(pos & 0xFF));
	outb(0x3D4, 0x0E);
	outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

/* Desenha um caractere diretamente no modo de texto VGA */
void vga_put_char(int x, int y, char c, uint8_t attributes)
{
	if (x >= VGA_WIDTH || y >= VGA_HEIGHT)
		return;

	uint8_t uc = (uint8_t)c;
	uint16_t pos = y * VGA_WIDTH + x;
	vga[pos] = uc | (attributes << 8);
}

/* Retorna um caractere de uma posição específica do modo de texto VGA */
uint16_t vga_get_char(int x, int y)
{
	if (x >= VGA_WIDTH || y >= VGA_HEIGHT)
		return 0;

	uint16_t pos = y * VGA_WIDTH + x;
	return vga[pos];
}

