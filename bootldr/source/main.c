/************************************
 * main.c                           *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>
#include "e820.h"
#include "stdio.h"
#include "string.h"
#include "disk.h"
#include "file.h"
#include "fat.h"
#include "vga.h"

char *kernel_file = "a:/system/boot/kernel.sys";
#define KERNEL_ADDR 0x10000

#define MAX_E820_ENTRIES 128

typedef struct boot_info {
	E820Entry *e820_table;
	int e820_entry_count;
} boot_info_t;

E820Entry e820_table[MAX_E820_ENTRIES];

boot_info_t boot_info;

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

	uint8_t *buf = (uint8_t *)KERNEL_ADDR;
	size_t file_size = seek(&f, UINT32_MAX);
	printf("Kernel tem %lu bytes\r\n", file_size);
	if (read(&f, file_size, buf) != file_size) {
		printf("Falha ao ler arquivo do kernel: %s\r\n", kernel_file);
		goto halt;
	}

	for (size_t i = 0; i < file_size; i++) {
		printf("%02X ", buf[i]);
		if ((i + 1) % 24 == 0)
			printf("\r\n");
	}
	printf("\r\n");

	boot_info.e820_entry_count = E820_get_table(e820_table, MAX_E820_ENTRIES);
	boot_info.e820_table = e820_table;

	__asm__ volatile (
		"MOV %0, %%EAX;"
		"JMP *%1"
		:: "r"(&boot_info), "r"(buf)
		: "eax"
	);

halt:
	printf("Sistema travado. Por favor, reinicie.\r\n");
	while (1);
}

