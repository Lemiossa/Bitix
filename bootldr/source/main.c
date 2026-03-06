/************************************
 * main.c                           *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "e820.h"
#include "util.h"
#include "disk.h"
#include "file.h"
#include "vesa.h"
#include "fat.h"
#include "vga.h"

#define HALT() __asm__ volatile ("cli;hlt");

#include "config.h"

#define MAX_E820_ENTRIES 128

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
	uint8_t vga_font_type;
} __attribute__((packed)) boot_info_t;

vbe_mode_info_t vbe_mode_info;

boot_info_t boot_info = {0};
e820_entry_t e820_table[MAX_E820_ENTRIES];

#define BAR_BACKGROUND 7
#define BAR_FOREGROUND 0
#define BAR_ATTR VGA_ATTR(BAR_FOREGROUND, BAR_BACKGROUND)
#define TERM_BACKGROUND 1
#define TERM_FOREGROUND 7
#define TERM_ATTR VGA_ATTR(TERM_FOREGROUND, TERM_BACKGROUND)

/* Trava o sistema */
void halt(void)
{
	printf("Sistema travado. Por favor, reinicie.\r\n");
	HALT();
}

/* Configura o VESA */
void set_vesa(void)
{
	uint16_t vesa_mode = 0x13;

	if (!video_modes[0]) {
		boot_info.graphics.mode = 0;
		printf("No video modes!\r\n");
		wait(1);
		return;
	}

	/* Tentar entrar nos modos */
	for (int i = 0; video_modes[i]; i++) {
		uint16_t mode = vesa_find_mode(video_modes[i]);
		if (mode == 0x13)
			continue;
		vesa_mode = mode;
		break;
	}

	if (vesa_set_mode(vesa_mode) != 0) {
		printf("Falha ao entrar no modo grafico!\r\n");
		boot_info.graphics.mode = 0;
		return;
	} else {
		if (vesa_get_mode_info(vesa_mode, &vbe_mode_info) != 0) {
			printf("Falha ao pegar informações do modo grafico!\r\n");
			halt();
		}
	}

	boot_info.graphics.mode = 1;
	boot_info.graphics.width =  vbe_mode_info.width;
	boot_info.graphics.height = vbe_mode_info.height;
	boot_info.graphics.pitch = vbe_mode_info.pitch;
	boot_info.graphics.bpp = vbe_mode_info.bpp;
	boot_info.graphics.red_mask = vbe_mode_info.red_mask;
	boot_info.graphics.red_position = vbe_mode_info.red_position;
	boot_info.graphics.green_mask = vbe_mode_info.green_mask;
	boot_info.graphics.green_position = vbe_mode_info.green_position;
	boot_info.graphics.blue_mask = vbe_mode_info.blue_mask;
	boot_info.graphics.blue_position = vbe_mode_info.blue_position;
	boot_info.graphics.framebuffer = vbe_mode_info.framebuffer;
}

/* Configura a fonte */
void set_font(void)
{
#ifdef FONT
	boot_info.vga_font_type = FONT;
	boot_info.vga_font = vga_get_font(FONT, NULL);
	if (!boot_info.vga_font) {
		printf("Falha ao pegar a fonte!\r\n");
		halt();
	}
#endif /* FONT */
}

/* Configura E820 */
void set_e820(void)
{
	boot_info.e820_table = &e820_table[0];
	boot_info.e820_entry_count = E820_get_table(boot_info.e820_table, MAX_E820_ENTRIES);

	if (boot_info.e820_entry_count == 0) {
		printf("Falha ao pegar mapa da memoria!\r\n");
		halt();
	}
}

/* Carrega o kernel */
void load_kernel(void)
{
	printf("Carregando %s...\r\n", KERNEL_FILE);
	file_t f;
	if (open(&f, KERNEL_FILE) != 0) {
		printf("Falha ao abrir arquivo do kernel: %s\r\n", KERNEL_FILE);
		halt();
	}

	size_t file_size = seek(&f, UINT32_MAX);
	seek(&f, 0);
	size_t ret = read(&f, file_size, (void *)(KERNEL_ADDR));
	if (ret != file_size) {
		printf("Falha ao ler arquivo do kernel: %s\r\n", KERNEL_FILE);
		halt();
	}
}

/* Func principal do bootloader */
int main()
{
	vga_clear(0x07);
	current_attributes = 0x70;
	printf("Bem vindo ao Bitix!\r\n");
	current_attributes = 0x07;

	wait(1);
	disk_detect();

	memset(&boot_info, 0, sizeof(boot_info_t));

	load_kernel();
	set_e820();
	set_vesa();
	set_font();

	uint32_t boot_info_loc = (uint32_t)&boot_info;
	__asm__ volatile (
		"MOV %0, %%EAX;"
		"JMP *%1"
		:: "r"(boot_info_loc), "r"(KERNEL_ADDR)
		: "eax"
	);
	halt();
}

