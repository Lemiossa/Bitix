/************************************
 * graphics.h                       *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#ifndef GRAPHICS_H
#define GRAPHICS_H
#include <stdint.h>

void put_pixel(int x, int y, uint32_t color);
uint32_t get_pixel(int x, int y);
void put_line(int x0, int y0, int x1, int y1, uint32_t color);

#endif /* GRAPHICS_H */
