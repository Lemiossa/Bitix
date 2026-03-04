/************************************
 * pmm.h                            *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#ifndef PMM_H
#define PMM_H
#include <stdint.h>
#include <stdbool.h>

#define PAGE_SIZE 4096

extern uint8_t *pmm_bitmap;
extern uint32_t pmm_total_pages;

bool pmm_init(void);
void pmm_mark_area(void *start_addr, void *end_addr);
void pmm_unmark_area(void *start_addr, void *end_addr);
void *pmm_alloc_page(void);
void pmm_free_page(void *pg);

#endif /* PMM_H */
