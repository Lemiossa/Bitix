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

typedef struct {
	uint16_t link, reserved0;
	uint32_t esp0;
	uint16_t ss0, reserved1;
	uint32_t esp1;
	uint16_t ss1, reserved2;
	uint32_t esp2;
	uint16_t ss2, reserved3;
	uint32_t cr3, eip, eflags;
	uint32_t eax, ecx, edx, ebx, esp, ebp, esi, edi;
	uint16_t es, reserved4;
	uint16_t cs, reserved5;
	uint16_t ss, reserved6;
	uint16_t ds, reserved7;
	uint16_t fs, reserved8;
	uint16_t gs, reserved9;
	uint16_t ldtr, reserved10;
	uint16_t reserved11, iomap_base;
} __attribute__((packed)) tss_t;

extern tss_t tss;

void gdt_set_entry(int entry, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags);
void gdt_init(void);
void tss_set_esp0(uint32_t esp0);

#endif /* GDT_H */
