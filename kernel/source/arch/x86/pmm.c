/************************************
 * pmm.c                            *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#include <stdint.h>
#include <stddef.h>
#include <terminal.h>

#include <boot.h>

#include <pmm.h>

static uint32_t *bitmap = NULL;

/* Inicializa pmm */
void pmm_init(void)
{
	uint32_t total_pages = 0;
	e820_entry_t *e820_table = boot_info.e820_table;
	for (int i = 0; i < boot_info.e820_entry_count; i++) {
		total_pages += e820_table[i].length / PAGE_SIZE;

	}

}
