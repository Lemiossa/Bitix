/************************************
 * pmm.c                            *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <terminal.h>

#include <boot.h>

#include <pmm.h>

static uint32_t *bitmap = NULL;
static uint32_t total_pages = 0;

extern uint8_t __end;
extern uint8_t __start;
extern uint8_t __bss_end;


/* Marca uma pagina como usada no PMM */
void pmm_mark_page(uint32_t page)
{
	if (!bitmap)
		return;

	uint32_t pos = page / 32;
	uint32_t bit = page % 32;
	bitmap[pos] |= (1 << bit);
}

/* Desmarca uma pagina como usada no PMM */
void pmm_unmark_page(uint32_t page)
{
	if (!bitmap)
		return;

	uint32_t pos = page / 32;
	uint32_t bit = page % 32;
	bitmap[pos] &= ~(1 << bit);
}

/* Retorna true se uma pagina específica está marcada como usada */
bool pmm_test_page(uint32_t page)
{
	if (!bitmap)
		return false;

	uint32_t pos = page / 32;
	uint32_t bit = page % 32;
	return bitmap[pos] & (1 << bit);
}

/* Inicializa pmm */
void pmm_init(void)
{
	bitmap = (uint32_t *)&__bss_end;

	e820_entry_t *e820_table = boot_info.e820_table;
	for (int i = 0; i < boot_info.e820_entry_count; i++) {
		total_pages += e820_table[i].length / PAGE_SIZE;
	}

	pmm_mark_page(0); /* IVT e BDA, pode ser que voltemos ao modo real alguma hora */

	for (int i = 0; i < boot_info.e820_entry_count; i++) {
		if (e820_table[i].type != 1) {
			for (uint32_t j = 0; j < e820_table[i].length; j++) {
				uint32_t page =  (e820_table[i].base + j) / PAGE_SIZE;
				pmm_unmark_page(page + j);
			}
		}
	}

	for (uint32_t i = (uint32_t)(&__start); i < (uint32_t)(&__end); i++) {
		uint32_t page = ((uint32_t)(&__start) + i) / PAGE_SIZE;
		pmm_mark_page(i);
	}

	/* Marcar paginas do bitmap */
	for (uint32_t i = (uint32_t)(&bitmap[0]); i < (uint32_t)(&bitmap[total_pages / 32]); i++) {
		uint32_t page = ((uint32_t)(&bitmap[0]) + i) / PAGE_SIZE;
		pmm_mark_page(i);
	}
}

/* Aloca uma pagina */
void *pmm_alloc_page(void)
{
	for (uint32_t i = 0; i < total_pages; i++) {
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
