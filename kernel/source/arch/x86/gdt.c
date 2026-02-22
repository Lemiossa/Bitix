/************************************
 * gdt.c                            *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#include "gdt.h"

gdtr_t gdtr;
gdt_entry_t gdt[GDT_ENTRIES];

/* Seta uma entrada na GDT */
void gdt_set_entry(int entry, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags)
{
	if (entry >= GDT_ENTRIES)
		return;

	gdt[entry].limit_low = limit & 0xFFFF;
	gdt[entry].base_low = base & 0xFFFF;
	gdt[entry].base_mid = (base >> 16) & 0xFF;
	gdt[entry].access = access;
	gdt[entry].flags = ((flags & 0x0F) << 4) | ((limit >> 16) & 0x0F);
	gdt[entry].base_high = (base >> 24) & 0xFF;
}

/* Inicializa GDT basica */
void gdt_init(void)
{
	gdt_set_entry(0, 0x00000000, 0x00000, 0b00000000, 0b0000); /* NULL   */
	gdt_set_entry(1, 0x00000000, 0xFFFFF, 0b10011010, 0b1100); /* CODE32 */
	gdt_set_entry(2, 0x00000000, 0xFFFFF, 0b10010010, 0b1100); /* DATA32 */

	gdtr.base = &gdt[0];
	gdtr.limit = GDT_ENTRIES * sizeof(gdt_entry_t) - 1;
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

