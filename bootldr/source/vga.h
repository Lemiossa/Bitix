/************************************
 * vga.h                            *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#ifndef VGA_H
#define VGA_H
#include <stdint.h>

#define VGA_WIDTH 80
#define VGA_HEIGHT 25

extern uint16_t vga_top_left_corner_x, vga_top_left_corner_y;
extern uint16_t vga_bottom_right_corner_x, vga_bottom_right_corner_y;
#define VGA_ATTR(fg, bg) (((fg) & 0x0F) | (((bg) & 0x0F) << 4))

void vga_set_cursor(uint16_t x, uint16_t y);
void vga_put_char(uint16_t x, uint16_t y, char c, uint8_t attributes);
void vga_put_string(uint16_t x, uint16_t y, char *s, uint8_t attributes);
uint16_t vga_get_char(uint16_t x, uint16_t y);
void vga_scroll(void);
void vga_clear(uint8_t attributes);

#endif /* VGA_H */
