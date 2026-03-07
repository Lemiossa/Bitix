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

/* Pega o PD atual */
static inline uint32_t *get_pd(void)
{
	if (paging_enabled) {
		return (uint32_t *)0xFFFFF000;
	}

	return (uint32_t *)get_cr3();
}

/* Pega uma PT específica */
static inline uint32_t *get_pt(uint32_t pd_index)
{
	if (paging_enabled) {
		return (uint32_t *)(0xFFC00000 + (pd_index * PAGE_SIZE));
	}

	return (uint32_t *)get_phys(get_pd()[pd_index]);
}

/* Pega o pd_index de um endereço virtual */
uint32_t get_pd_index(uint32_t virt)
{
	return (virt >> 22) & 0x3FF;
}

/* Pega o pt_index de um endereço virtual */
uint32_t get_pt_index(uint32_t virt)
{
	return (virt >> 12) & 0x3FF;
}

/* Mapeia uma pagina */
bool vmm_map(uint32_t phys, uint32_t virt, uint32_t flags)
{
	uint32_t pd_index = get_pd_index(virt);
	uint32_t pt_index = get_pt_index(virt);

	uint32_t *pd = get_pd();

	if (!pd)
		return false;

	if (!(pd[pd_index] & PAGE_PRESENT)) {
		uint32_t *new_pt = pmm_alloc_page();
		if (!new_pt)
			return false;

		pd[pd_index] = make_entry((uint32_t)new_pt, PAGE_PRESENT | PAGE_WRITE | (flags & PAGE_USER));
		if (paging_enabled) {
			invlpg(0xFFC00000 + (pd_index * PAGE_SIZE));
			memset((void *)(0xFFC00000 + (pd_index * PAGE_SIZE)), 0, PAGE_SIZE);
		} else {
			memset(new_pt, 0, PAGE_SIZE);
		}
	}

	uint32_t *pt = get_pt(pd_index);
	pt[pt_index] = make_entry(phys, flags | PAGE_PRESENT);

	if (paging_enabled)
		invlpg(virt);

	return true;
}

/* Desmapeia uma pagina */
bool vmm_unmap(uint32_t virt)
{
	uint32_t pd_index = get_pd_index(virt);
	uint32_t pt_index = get_pt_index(virt);

	uint32_t *pd = get_pd();

	if (!pd)
		return false;

	if (!(pd[pd_index] & PAGE_PRESENT))
		return true;

	uint32_t *pt = get_pt(pd_index);
	pt[pt_index] = 0;

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

	kernel_pd[1023] = make_entry((uint32_t)kernel_pd, PAGE_PRESENT | PAGE_WRITE);

	set_cr3((uint32_t)kernel_pd);
	for (uint32_t i = 0; i < 0x00400000; i += PAGE_SIZE) {
		if (!vmm_map(i, i, PAGE_WRITE | PAGE_PRESENT))
			continue;
	}

	set_cr0(get_cr0() | CR0_PG);
	paging_enabled = true;
	return true;
}

