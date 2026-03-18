/************************************
 * boot.h                           *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#ifndef BOOT_H
#define BOOT_H
#include <e820.h>
#include <stdint.h>

typedef struct graphics_info
{
	int mode; /* 1 = modo vesa */
	int width, height;
	int pitch;
	int bpp;
	uint32_t framebuffer;
	uint16_t red_mask, red_position;
	uint16_t green_mask, green_position;
	uint16_t blue_mask, blue_position;
} graphics_info_t;

typedef struct boot_info
{
	graphics_info_t graphics;
	e820_entry_t *e820_table;
	int e820_entry_count;
	uint32_t boot_signature;
} __attribute__((packed)) boot_info_t;

extern boot_info_t boot_info;

#endif /* BOOT_H */
