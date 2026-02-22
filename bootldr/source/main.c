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
#include "fat.h"
#include "vga.h"

#define HALT() __asm__ volatile ("cli;hlt");

#define kernel_file "/system/boot/kernel.sys"
#define KERNEL_ADDR 0x10000

#define MAX_E820_ENTRIES 128

typedef struct boot_info {
	e820_entry_t *e820_table;
	int e820_entry_count;
} __attribute__((packed)) boot_info_t;

boot_info_t boot_info;
e820_entry_t e820_table[MAX_E820_ENTRIES];

#define BAR_BACKGROUND 7
#define BAR_FOREGROUND 0
#define BAR_ATTR VGA_ATTR(BAR_FOREGROUND, BAR_BACKGROUND)
#define TERM_BACKGROUND 1
#define TERM_FOREGROUND 7
#define TERM_ATTR VGA_ATTR(TERM_FOREGROUND, TERM_BACKGROUND)

/* Imprime a UI do bootloader */
void bootloader_ui(void)
{
	vga_clear(TERM_ATTR);

	for (uint16_t i = 0; i < VGA_WIDTH; i++) {
		vga_put_char(i, 0, ' ', BAR_ATTR);
	}
	vga_put_string(1, 0, "Carregador do Bitix!", BAR_ATTR);

	for (uint16_t i = 0; i < VGA_WIDTH; i++) {
		vga_put_char(i, VGA_HEIGHT - 1, ' ', BAR_ATTR);
	}
	vga_put_string(1, VGA_HEIGHT - 1, "Criado por Matheus Leme Da Silva - Brasil", BAR_ATTR);

	current_attributes = TERM_ATTR;
	cursor_x = 0;
	cursor_y = 1;

	vga_top_left_corner_x = 0;
	vga_top_left_corner_y = 1;
	vga_bottom_right_corner_x = VGA_WIDTH;
	vga_bottom_right_corner_y = VGA_HEIGHT - 2;
}

/* Func principal do bootloader */
int main()
{
	vga_clear(0x07);
	current_attributes = 0x70;
	printf("Bem vindo ao Bitix!\r\n");
	current_attributes = 0x07;

	wait(1);

	bootloader_ui();
	disk_detect();

	for (int i = 3; i > 0; i--) {
		printf("\rCarregando em %d segundos...", i);
		wait(1);
	}
	printf("\r\n");

	printf("Carregando %s...\r\n", kernel_file);

	file_t f;
	if (open(&f, kernel_file) != 0) {
		printf("Falha ao abrir arquivo do kernel: %s\r\n", kernel_file);
		goto halt;
	}

	size_t file_size = seek(&f, UINT32_MAX);
	seek(&f, 0);
	size_t ret = read(&f, file_size, (void *)(KERNEL_ADDR));
	if (ret != file_size) {
		printf("Falha ao ler arquivo do kernel: %s\r\n", kernel_file);
		goto halt;
	}

	boot_info.e820_table = &e820_table[0];
	boot_info.e820_entry_count = E820_get_table(boot_info.e820_table, MAX_E820_ENTRIES);

	if (boot_info.e820_entry_count == 0) {
		printf("Falha ao pegar mapa da memoria!\r\n");
		goto halt;
	}

	uint32_t boot_info_loc = (uint32_t)&boot_info;
	__asm__ volatile (
		"MOV %0, %%EAX;"
		"JMP *%1"
		:: "r"(boot_info_loc), "r"(KERNEL_ADDR)
		: "eax"
	);

halt:
	printf("Sistema travado. Por favor, reinicie.\r\n");
	HALT();
}

