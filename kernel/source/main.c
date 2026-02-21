/************************************
 * main.c                           *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#include <stdint.h>
#include <stdio.h>
#include "vga.h"
#include "terminal.h"

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

#define GDT_MAX_ENTRIES 256

typedef struct gdtr {
	uint16_t limit;
	void *base;
} __attribute__((packed)) gdtr_t;

typedef struct gdt_entry {
	uint16_t limit_low;
	uint16_t base_low;
	uint8_t base_mid;
	uint8_t access;
	uint8_t flags;
	uint8_t base_high;
} __attribute__((packed)) gdt_entry_t;

gdtr_t gdtr;
gdt_entry_t gdt[GDT_MAX_ENTRIES];
int gdt_entry_count = 0;

/* Adiciona uma nova entrada na GDT */
void gdt_add_entry(uint32_t base, uint32_t limit, uint8_t access, uint8_t flags)
{
	if (gdt_entry_count >= GDT_MAX_ENTRIES)
		return;
	flags &= 0x0F;

	gdt[gdt_entry_count].limit_low = limit & 0xFFFF;
	gdt[gdt_entry_count].base_low = base & 0xFFFF;
	gdt[gdt_entry_count].base_mid = (base >> 16) & 0xFF;
	gdt[gdt_entry_count].access = access;
	gdt[gdt_entry_count].flags = (flags << 4) | ((limit >> 16) & 0x0F);
	gdt[gdt_entry_count].base_high = (base >> 24) & 0xFF;
	gdt_entry_count++;

}

/* Inicializa GDT basica */
void gdt_init(void)
{
	gdt_add_entry(0x00000000, 0x00000, 0b00000000, 0b0000); /* NULL   */
	gdt_add_entry(0x00000000, 0xFFFFF, 0b10011010, 0b1100); /* CODE32 */
	gdt_add_entry(0x00000000, 0xFFFFF, 0b10010010, 0b1100); /* DATA32 */

	gdtr.base = &gdt[0];
	gdtr.limit = GDT_MAX_ENTRIES * sizeof(gdt_entry_t) - 1;
	__asm__ volatile(
			"LGDT %0;"
			"LJMP $0x08, $flush;"
			"flush:"
			"MOV $0x10, %%AX;"
			"MOV %%AX, %%DS;"
			"MOV %%AX, %%ES;"
			"MOV %%AX, %%FS;"
			"MOV %%AX, %%GS;"
			"MOV %%AX, %%SS;"
			:: "m"(gdtr)
	);
}

/* Func principal */
void kernel_main(boot_info_t *boot_info)
{
	vga_clear(0x07);
	gdt_init();
	printf("Kernel iniciado!\r\n");

	for (int count = 0; count < boot_info->e820_entry_count; count++) {
		e820_entry_t entry = boot_info->e820_table[count];
		printf("E820_table[%d]: 0x%08X-0x%08X:%d\r\n",
				count, (uint32_t)entry.base, (uint32_t)entry.length, entry.type);
	}

	while (1);
}
