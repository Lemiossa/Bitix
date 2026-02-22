/************************************
 * idt.h                            *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#ifndef IDT_H
#define IDT_H
#include <stdint.h>

#define IDT_ENTRIES 256

typedef struct idtr {
	uint16_t limit;
	void *base;
} __attribute__((packed)) idtr_t;

typedef struct idt_entry {
	uint16_t addr_low;
	uint16_t selector;
	uint8_t res;
	uint8_t attr;
	uint16_t addr_high;
} __attribute__((packed)) idt_entry_t;

typedef struct intr_frame {
	uint32_t ds, es, fs, gs;
	uint32_t edi, esi, ebp, original_esp, ebx, edx, ecx, eax;
	uint32_t int_no, err_code;
	uint32_t eip, cs, eflags;
	uint32_t user_esp, user_ss;
} __attribute__((packed)) intr_frame_t;

void idt_set_intr(int entry, void (*intr)(void), uint16_t selector);
void idt_set_trap(int entry, void (*trap)(void), uint16_t selector);
void idt_init(void);

#endif /* IDT_H */

