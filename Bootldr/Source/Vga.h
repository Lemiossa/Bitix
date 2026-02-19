/************************************
 * Vga.h                            *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#ifndef VGA_H
#define VGA_H
#include <stdint.h>

#define VGA_WIDTH 80
#define VGA_HEIGHT 25

void vga_set_cursor(uint16_t x, uint16_t y);
void vga_put_char(uint16_t x, uint16_t y, char c, uint8_t attributes);
uint16_t vga_get_char(uint16_t x, uint16_t y);
void vga_scroll(void);
void vga_clear(uint8_t attributes);

#endif /* VGA_H */
