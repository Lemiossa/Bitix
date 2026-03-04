/************************************
 * pmm.c                            *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <terminal.h>

#include <boot.h>

#include <pmm.h>

uint8_t *pmm_bitmap = NULL;
uint32_t pmm_total_pages = 0;

extern uint8_t __end;
extern uint8_t __start;
extern uint8_t __bss_end;

/* Marca uma pagina como usada no PMM */
void pmm_mark_page(uint32_t page)
{
	if (!pmm_bitmap)
		return;

	uint32_t pos = page / 8;
	uint32_t bit = page % 8;
	pmm_bitmap[pos] |= (1 << bit);
}

/* Desmarca uma pagina como usada no PMM */
void pmm_unmark_page(uint32_t page)
{
	if (!pmm_bitmap)
		return;

	uint32_t pos = page / 8;
	uint32_t bit = page % 8;
	pmm_bitmap[pos] &= ~(1 << bit);
}

/* Retorna true se uma pagina específica está marcada como usada */
bool pmm_test_page(uint32_t page)
{
	if (!pmm_bitmap)
		return false;

	uint32_t pos = page / 8;
	uint32_t bit = page % 8;
	return pmm_bitmap[pos] & (1 << bit);
}

/* Marca uma area */
void pmm_mark_area(void *start_addr, void *end_addr)
{
	if (!pmm_bitmap)
		return;

	uint32_t start = (uint32_t)start_addr;
	uint32_t end = (uint32_t)end_addr;

	start &= ~(PAGE_SIZE - 1);
	end = (end + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);

	uint32_t start_page = start / PAGE_SIZE;
	uint32_t end_page = end / PAGE_SIZE;

	for (uint32_t page = start_page; page < end_page; page++)
		pmm_mark_page(page);
}

/* Desmarca uma grande area */
void pmm_unmark_area(void *start_addr, void *end_addr)
{
	if (!pmm_bitmap)
		return;

	uint32_t start = (uint32_t)start_addr;
	uint32_t end = (uint32_t)end_addr;

	start &= ~(PAGE_SIZE - 1);
	end = (end + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);

	uint32_t start_page = start / PAGE_SIZE;
	uint32_t end_page = end / PAGE_SIZE;

	for (uint32_t page = start_page; page < end_page; page++)
		pmm_unmark_page(page);
}

/* Inicializa pmm */
bool pmm_init(void)
{
	pmm_bitmap = (uint8_t *)&__bss_end;


	e820_entry_t *e820_table = boot_info.e820_table;
	for (int i = 0; i < boot_info.e820_entry_count; i++) {
		pmm_total_pages += e820_table[i].length / PAGE_SIZE;
	}

	memset(pmm_bitmap, 0, pmm_total_pages / 8);
	pmm_mark_page(0); /* IVT e BDA, pode ser que voltemos ao modo real alguma hora */

	for (int i = 0; i < boot_info.e820_entry_count; i++) {
		if (e820_table[i].type != 1) {
			pmm_mark_area((void *)(uint32_t)e820_table[i].base, (void *)((uint32_t)e820_table[i].base + (uint32_t)e820_table[i].length));
		}
	}

	pmm_mark_area(&__start, &__end);
	pmm_mark_area(&pmm_bitmap[0], &pmm_bitmap[pmm_total_pages]);
	return true;
}

/* Aloca uma pagina */
void *pmm_alloc_page(void)
{
	for (uint32_t i = 0; i < pmm_total_pages; i++) {
		uint32_t pos = i / 32;

		if (pmm_bitmap[pos] == 0xFF)
			continue;

		if (pmm_test_page(i) == false) {
			pmm_mark_page(i);
			return (void *)(i * PAGE_SIZE);
		}
	}

	return NULL;
}

/* Libera uma pagina */
void pmm_free_page(void *pg)
{
	uint32_t page = (uint32_t)pg / PAGE_SIZE;
	pmm_unmark_page(page);
}
