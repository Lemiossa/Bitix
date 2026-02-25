/************************************
 * graphics.c                       *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#include <stdint.h>
#include <graphics.h>
#include <boot.h>
#include <terminal.h>

/* Desenha um PIXEL no modo grafico */
void put_pixel(int x, int y, uint32_t color)
{
	if (x >= boot_info.graphics.width || y >= boot_info.graphics.height ||
			boot_info.graphics.mode == 0 || x < 0 || y < 0)
		return;

	uint32_t val;
	if (boot_info.graphics.bpp > 8) {
		uint8_t cr = (color >> 16) & 0xFF;
		uint8_t cg = (color >> 8) & 0xFF;
		uint8_t cb = (color >> 0) & 0xFF;

		uint8_t r = (cr * ((1 << boot_info.graphics.red_mask) - 1)) / 255;
		uint8_t g = (cg * ((1 << boot_info.graphics.green_mask) - 1)) / 255;
		uint8_t b = (cb * ((1 << boot_info.graphics.blue_mask) - 1)) / 255;

		val = r << boot_info.graphics.red_position;
		val |= g << boot_info.graphics.green_position;
		val |= b << boot_info.graphics.blue_position;
	} else {
		val = color;
	}

	uint8_t* fb = (uint8_t*)boot_info.graphics.framebuffer;
	uint32_t bytes = boot_info.graphics.bpp / 8;
	uint32_t offset = y * boot_info.graphics.pitch + x * bytes;

	for (uint32_t i = 0; i < bytes; i++) {
		fb[offset + i] = (val >> (i * 8)) & 0xFF;
	}
}

/* Retorna um pixel no modo grafico */
uint32_t get_pixel(int x, int y)
{
	if (x >= boot_info.graphics.width || y >= boot_info.graphics.height ||
			boot_info.graphics.mode == 0 || x < 0 || y < 0)
		return 0;

	uint32_t pos = y * boot_info.graphics.pitch + (x * (boot_info.graphics.bpp/8)) + boot_info.graphics.framebuffer;

	uint32_t val = *(uint32_t *)pos;
	uint32_t ret = 0;
	if (boot_info.graphics.bpp > 8) {
		uint8_t r = 0, g = 0, b = 0;

		r = (val >> boot_info.graphics.red_position) & ((1 << boot_info.graphics.red_mask) - 1);
		g = (val >> boot_info.graphics.green_position) & ((1 << boot_info.graphics.green_mask) - 1);
		b = (val >> boot_info.graphics.blue_position) & ((1 << boot_info.graphics.blue_mask) - 1);

		ret = r | (g << 8) | (b << 16);
	} else {
		ret = val;
	}

	return ret;
}

/* Desenha uma reta no modo grafico */
/* Metodo Bresenham simplificado */
void put_line(int x0, int y0, int x1, int y1, uint32_t color)
{
	int dx = x1 - x0;
	int dy = y1 - y0;
	int d = (2 * dy) - dx;
	int y = y0;

	for (int x = x0; x <= x1; x++) {
		put_pixel(x, y, color);
		if (d > 0) {
			y++;
			d += 2 * (dy - dx);
		} else {
			d += 2 * dy;
		}
	}
}

