/************************************
 * graphics.h                       *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#ifndef GRAPHICS_H
#define GRAPHICS_H
#include <stdint.h>

#define RGB(r, g, b) /* Cria um valor uint32_t com 0xRRGGBB */\
	(uint32_t)( \
			(((r) & 0xFF) << 16) | /* Vermelho */ \
			(((g) & 0xFF) << 8) | /* Verde */ \
			((b & 0xFF)) /* Azul */ \
			)

void put_pixel(int x, int y, uint32_t color);
uint32_t get_pixel(int x, int y);
void put_line(int x0, int y0, int x1, int y1, uint32_t color);

#endif /* GRAPHICS_H */
