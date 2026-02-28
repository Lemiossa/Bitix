/************************************
 * pmm.h                            *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#ifndef PMM_H
#define PMM_H

#define PAGE_SIZE 4096

void pmm_init(void);
void *pmm_alloc_page(void);
void pmm_free_page(void *pg);

#endif /* PMM_H */
