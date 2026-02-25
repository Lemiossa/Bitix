/************************************
 * boot.h                           *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#ifndef BOOT_H
#define BOOT_H
#include <stdint.h>

typedef struct e820_entry {
	uint64_t base;
	uint64_t length;
	uint32_t type;
	uint32_t attr;
} __attribute__((packed)) e820_entry_t;

typedef struct graphics_info {
	int mode; /* 1 = modo vesa */
	int width, height;
	int pitch;
	int bpp;
	uint32_t framebuffer;
	uint16_t red_mask, red_position;
	uint16_t green_mask, green_position;
	uint16_t blue_mask, blue_position;
} graphics_info_t;

typedef struct boot_info {
	e820_entry_t *e820_table;
	int e820_entry_count;
	graphics_info_t graphics;
	uint8_t *vga_font;
} __attribute__((packed)) boot_info_t;

extern boot_info_t boot_info;

#endif /* BOOT_H */
