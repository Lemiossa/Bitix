/************************************
 * vmm.c                            *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <terminal.h>
#include <asm.h>
#include <idt.h>
#include <pmm.h>
#include <vmm.h>
#include <panic.h>
#include <sched.h>

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
	if (paging_enabled)
	{
		return (uint32_t *)0xFFFFF000;
	}

	return (uint32_t *)get_cr3();
}

/* Pega uma PT específica */
static inline uint32_t *get_pt(uint32_t pd_index)
{
	if (paging_enabled)
	{
		return (uint32_t *)(0xFFC00000 + (pd_index * PAGE_SIZE));
	}

	return (uint32_t *)get_phys(get_pd()[pd_index]);
}

/* Pega o pd_index de um endereço virtual */
static inline uint32_t get_pd_index(uint32_t virt)
{
	return (virt >> 22) & 0x3FF;
}

/* Pega o pt_index de um endereço virtual */
static inline uint32_t get_pt_index(uint32_t virt)
{
	return (virt >> 12) & 0x3FF;
}

/* Retorna true se um endereço virtual está presente */
bool vmm_virt_is_present(uint32_t virt)
{
	uint32_t pd_index = get_pd_index(virt);
	uint32_t pt_index = get_pt_index(virt);

	uint32_t *pd = get_pd();

	if (!pd)
		return false;

	if (!(pd[pd_index] & PAGE_PRESENT))
		return false;

	uint32_t *pt = get_pt(pd_index);

	return pt[pt_index] & PAGE_PRESENT;
}

/* Handler de page fault */
void page_fault_handler(intr_frame_t *f)
{
	exit(-1);
	cli();
	printf("Falha de pagina!\r\n");

	printf("CR2: 0x%08X\r\n", get_cr2());

	printf("EAX: 0x%08X ", f->eax);
	printf("EBX: 0x%08X\r\n", f->ebx);
	printf("ECX: 0x%08X ", f->ecx);
	printf("EDX: 0x%08X\r\n", f->edx);
	printf("EBP: 0x%08X ", f->ebp);
	printf("ESI: 0x%08X\r\n", f->esi);
	printf("EDI: 0x%08X\r\n\r\n", f->edi);
	printf("EIP: 0x%08X\r\n", f->eip);
	printf("CS:  0x%08X ", f->cs);
	printf("DS:  0x%08X\r\n", f->ds);
	printf("ES:  0x%08X ", f->es);
	printf("FS:  0x%08X\r\n", f->fs);
	printf("GS:  0x%08X\r\n", f->gs);
}

/* Retorna o endereço fisico de um virtual atualmente */
uint32_t vmm_get_phys(uint32_t virt)
{
	uint32_t pd_index = get_pd_index(virt);
	uint32_t pt_index = get_pt_index(virt);

	uint32_t *pd = get_pd();

	if (!pd)
		return 0;

	if (!(pd[pd_index] & PAGE_PRESENT))
		return 0;

	uint32_t *pt = get_pt(pd_index);
	if (!(pt[pt_index] & PAGE_PRESENT))
		return 0;

	return get_phys(pt[pt_index]);
}

/* Cria uma nova PD */
/* Retorna o endereço virtual dela */
uint32_t vmm_create_pd(void)
{
	uint32_t pd = (uint32_t)pmm_alloc_page();
	if (!pd)
		return 0;

	uint32_t virt = vmm_get_free_virt();
	if (!virt)
	{
		pmm_free_page((void *)pd);
		return 0;
	}

	if (!vmm_map(pd, virt, PAGE_PRESENT | PAGE_WRITE))
	{
		pmm_free_page((void *)pd);
		return 0;
	}

	return virt;
}

/* Mapeia uma pagina */
bool vmm_map(uint32_t phys, uint32_t virt, uint32_t flags)
{
	uint32_t pd_index = get_pd_index(virt);
	uint32_t pt_index = get_pt_index(virt);

	uint32_t *pd = get_pd();

	if (!pd)
		return false;

	if (!(pd[pd_index] & PAGE_PRESENT))
	{
		uint32_t *new_pt = pmm_alloc_page();
		if (!new_pt)
			return false;

		pd[pd_index] = make_entry((uint32_t)new_pt, PAGE_PRESENT | PAGE_WRITE |
														(flags & PAGE_USER));
		if (paging_enabled)
		{
			invlpg(0xFFC00000 + (pd_index * PAGE_SIZE));
			memset((void *)(0xFFC00000 + (pd_index * PAGE_SIZE)), 0, PAGE_SIZE);
		}
		else
		{
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

/* Retorna um endereço virtual temporario na area do kernel */
/* Por enquanto é lento, mas funciona */
/* 0 é inválido */
uint32_t vmm_get_free_virt(void)
{
	uint32_t ptr = 0xC0000000;
	while (ptr < 0xFFC00000)
	{
		if (!vmm_virt_is_present(ptr))
			break;
		ptr += PAGE_SIZE;
	}

	return ptr;
}

/* Retorna um endereço virtual temporario na area do usuario */
/* Por enquanto é lento, mas funciona */
/* 0 é inválido */
uint32_t vmm_get_free_virt_user(void)
{
	uint32_t ptr = 0x00400000;
	while (ptr < 0xC0000000)
	{
		if (!vmm_virt_is_present(ptr))
			break;
		ptr += PAGE_SIZE;
	}

	return ptr;
}

/* Inicializa o virtual memory manager */
void vmm_init(void)
{
	kernel_pd = pmm_alloc_page();
	if (!kernel_pd)
		panic("VMM: Falha ao alocar memoria para o diretorio de paginas do kernel\r\n");

	memset(kernel_pd, 0, PAGE_SIZE);

	kernel_pd[1023] =
		make_entry((uint32_t)kernel_pd, PAGE_PRESENT | PAGE_WRITE);

	set_cr3((uint32_t)kernel_pd);
	for (uint32_t i = 0; i < 0x00400000; i += PAGE_SIZE)
	{
		if (!vmm_map(i, i, PAGE_WRITE | PAGE_PRESENT))
			continue;
	}

	set_cr0(get_cr0() | CR0_PG);
	paging_enabled = true;
	idt_set_trap(14, page_fault_handler, 0x08);
}
