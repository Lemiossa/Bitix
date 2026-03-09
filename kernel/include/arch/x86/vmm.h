/************************************
 * vmm.h                            *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#ifndef VMM_H
#define VMM_H
#include <stdbool.h>
#include <stdint.h>

#define PAGE_PRESENT (1 << 0)
#define PAGE_WRITE (1 << 1)
#define PAGE_USER (1 << 2)

extern uint32_t *kernel_pd;
extern bool paging_enabled;

uint32_t vmm_get_free_virt_user(void);
uint32_t vmm_get_phys(uint32_t virt);
bool vmm_map(uint32_t phys, uint32_t virt, uint32_t flags);
bool vmm_unmap(uint32_t virt);
uint32_t vmm_get_free_virt(void);
bool vmm_init(void);

#endif /* VMM_H */
