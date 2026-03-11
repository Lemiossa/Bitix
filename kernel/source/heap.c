/************************************
 * heap.c                           *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <pmm.h>
#include <vmm.h>

typedef struct heap_block {
	uint32_t length;
	bool free;
	struct heap_block *prev;
	struct heap_block *next;
} heap_block_t;

#define HEAP_START_SIZE 0x200000
#define HEAP_START 0x90000000

heap_block_t *start = NULL;
uint32_t heap_size = 0;

/* Procura o primeiro bloco livre no heap com N bytes de tamanho */
static heap_block_t *find_free_block(size_t n)
{
	heap_block_t *current = start;

	while (current) {
		if (current->free && current->length >= n)
			return current;
		current = current->next;
	}

	return NULL;
}

/* Transforma o endereço de um bloco em endereço de payload */
static void *block_to_payload(heap_block_t *block)
{
	return (void *)((uint8_t *)block + sizeof(heap_block_t));
}

/* Transforma o endereço de um payload em endereço de bloco */
static heap_block_t *payload_to_block(void *payload)
{
	return (heap_block_t *)((uint8_t *)payload - sizeof(heap_block_t));
}

/* Inicializa HEAP */
void heap_init(void)
{
	uint32_t pages = HEAP_START_SIZE / PAGE_SIZE;
	for (uint32_t i = 0; i < pages; i++) {
		uint32_t phys = (uint32_t)pmm_alloc_page();
		if (!phys)
			break;

		uint32_t page_addr = HEAP_START + (i * PAGE_SIZE);

		if (vmm_virt_is_present(page_addr)) /* Pra não atingir outras coisas */
			break;

		if (!vmm_map(phys, page_addr, PAGE_PRESENT | PAGE_WRITE))
			break;

		heap_size += PAGE_SIZE;
	}

	start = (heap_block_t *)HEAP_START;

	start->free = true;
	start->length = heap_size - sizeof(heap_block_t);
	start->next = 0;
	start->prev = 0;
}

/* Aloca n bytes de memoria no heap */
void *alloc(size_t n)
{
	heap_block_t *free_blk = find_free_block(n);
	if (!free_blk)
		return NULL;

	if (free_blk->length > n + sizeof(heap_block_t) + 1) {
		size_t size = free_blk->length - n - sizeof(heap_block_t);
		heap_block_t *new_block = (heap_block_t *)((uint8_t *)block_to_payload(free_blk) + n);
		new_block->free = true;
		new_block->length = size;
		new_block->prev = free_blk;
		new_block->next = free_blk->next;

		if (new_block->next)
			new_block->next->prev = new_block;

		free_blk->next = new_block;
		free_blk->length = n;
	}

	free_blk->free = false;

	return block_to_payload(free_blk);
}

/* Libera um bloco de memoria */
void free(void *p)
{
	if (!p)
		return;
	heap_block_t *b = payload_to_block(p);

	if (b->free) /* Double free */
		return;

	if (b->next && b->next->free) {
		b->length += b->next->length + sizeof(heap_block_t);
		if (b->next->next)
			b->next->next->prev = b;
	}

	if (b->prev && b->prev->free) {
		b->prev->length += b->length + sizeof(heap_block_t);
		if (b->next)
			b->next->prev = b->prev;
		b = b->prev;
	}


	b->free = true;
}
