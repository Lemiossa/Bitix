/************************************
 * main.c                           *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>
#include "e820.h"
#include "string.h"
#include "util.h"
#include "disk.h"
#include "file.h"
#include "fat.h"
#include "vga.h"

char *kernel_file = "/system/boot/kernel.sys";
#define KERNEL_ADDR 0x100000

#define MAX_E820_ENTRIES 128

typedef struct boot_info {
	e820_entry_t *e820_table;
	int e820_entry_count;
} __attribute__((packed)) boot_info_t;

boot_info_t boot_info;
e820_entry_t e820_table[MAX_E820_ENTRIES];

/* Func principal do bootloader */
int main(void)
{
	vga_clear(0x07);
	printf("Ola mundo!\r\n");

	disk_dettect();

	if (fat_configure(boot_disk, 0) != 0) { /* FLOPPY */
		printf("Falha ao inicializar sistema FAT\r\n");
		goto halt;
	}

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
	while (1);
}

