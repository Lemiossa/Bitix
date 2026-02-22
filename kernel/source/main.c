/************************************
 * main.c                           *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#include <stdint.h>
#include <stdio.h>
#include <vga.h>
#include <terminal.h>

#include <gdt.h>
#include <idt.h>

typedef struct e820_entry {
	uint64_t base;
	uint64_t length;
	uint32_t type;
	uint32_t attr;
} __attribute__((packed)) e820_entry_t;

typedef struct boot_info {
	e820_entry_t *e820_table;
	int e820_entry_count;
} __attribute__((packed)) boot_info_t;

/* Func principal */
void kernel_main(boot_info_t *boot_info)
{
	vga_clear(0x17);
	/*gdt_init();
	idt_init();*/

	printf("Kernel iniciado!\r\n");

	for (int count = 0; count < boot_info->e820_entry_count; count++) {
		e820_entry_t entry = boot_info->e820_table[count];
		printf("E820_table[%d]: 0x%08X-0x%08X:%d\r\n",
				count, (uint32_t)entry.base, (uint32_t)entry.length, entry.type);
	}

	while (1);
}
