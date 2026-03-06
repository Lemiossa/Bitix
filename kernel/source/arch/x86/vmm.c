/************************************
 * vmm.c                            *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include <string.h>

#include <asm.h>

#include <pmm.h>
#include <vmm.h>

uint32_t *kernel_pd = NULL;
bool paging_enabled = false;

/* Cria uma entrada paging  */
static inline uint32_t make_entry(uint32_t addr, uint32_t flags)
{
	return (addr & 0xFFFFF000) | (flags & 0xFFF);
}

/* Retorna o endereço fisico de uma entrada paging */
static inline uint32_t get_phys(uint32_t entry)
{
	return entry & 0xFFFFF000;
}

/* Retorna as flags de uma entrada paging */
static inline uint32_t get_flags(uint32_t entry)
{
	return entry & 0xFFF;
}

/* Mapeia uma pagina */
bool vmm_map(uint32_t *pd, uint32_t phys, uint32_t virt, uint32_t flags)
{
	if (!pd)
		return false;

	uint32_t pd_index = (virt >> 22) & 0x3FF;
	uint32_t pt_index = (virt >> 12) & 0x3FF;

	if (!(pd[pd_index] & PAGE_PRESENT)) {
		uint32_t *new_pt = pmm_alloc_page();
		if (!new_pt)
			return false;

		memset(new_pt, 0, PAGE_SIZE);
		pd[pd_index] = make_entry((uint32_t)new_pt, PAGE_PRESENT | PAGE_WRITE | (flags & PAGE_USER));
	}

	uint32_t *pt = (uint32_t *)get_phys(pd[pd_index]);
	pt[pt_index] = make_entry(phys, flags | PAGE_PRESENT);

	if (paging_enabled)
		invlpg(virt);

	return true;
}

/* Inicializa o virtual memory manager */
bool vmm_init(void)
{
	kernel_pd = pmm_alloc_page();
	if (!kernel_pd)
		return false;

	memset(kernel_pd, 0, PAGE_SIZE);

	for (uint32_t i = 0; i < 0x00400000; i += PAGE_SIZE) {
		vmm_map(kernel_pd, i, i, PAGE_WRITE | PAGE_PRESENT);
	}

	set_cr3((uint32_t)kernel_pd);
	set_cr0(get_cr0() | CR0_PG);
	paging_enabled = true;
	return true;
}

