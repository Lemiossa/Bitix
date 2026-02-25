/************************************
 * gdt.h                            *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#ifndef GDT_H
#define GDT_H
#include <stdint.h>

#define GDT_ENTRIES 256

typedef struct gdtr {
	uint16_t limit;
	void *base;
} __attribute__((packed)) gdtr_t;

typedef struct gdt_entry {
	uint16_t limit_low;
	uint16_t base_low;
	uint8_t base_mid;
	uint8_t access;
	uint8_t flags;
	uint8_t base_high;
} __attribute__((packed)) gdt_entry_t;

void gdt_set_entry(int entry, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags);
void gdt_init(void);

#endif /* GDT_H */
