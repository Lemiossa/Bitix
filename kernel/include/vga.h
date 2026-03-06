/************************************
 * vga.h                            *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#ifndef VGA_H
#define VGA_H
#include <stdint.h>

#define VGA_WIDTH 80
#define VGA_HEIGHT 25

void vga_set_cursor(int x, int y);
uint16_t vga_get_cursor(void);
void vga_set_cursor_shape(uint8_t start, uint8_t end);
void vga_disable_cursor(void);
void vga_put_char(int x, int y, char c, uint8_t attributes);
uint16_t vga_get_char(int x, int y);

#endif /* VGA_H */
