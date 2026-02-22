/************************************
 * idt.c                            *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#include <stdint.h>
#include <terminal.h>
#include <idt.h>

#define INTERRUPT_GATE32 0x8E
#define TRAP_GATE32 0x8F

idtr_t idtr;
idt_entry_t idt[IDT_ENTRIES];

/* Seta uma entrada na IDT */
void idt_set_entry(int entry, uint32_t addr, uint16_t selector, uint8_t attr)
{
	if (entry >= IDT_ENTRIES)
		return;

	idt[entry].addr_low = addr & 0xFFFF;
	idt[entry].selector = selector;
	idt[entry].res = 0;
	idt[entry].attr = attr;
	idt[entry].addr_high = (addr >> 16) & 0xFFFF;
}

/* Seta uma interrupt gate na IDT */
void idt_set_intr(int entry, void (*intr)(void), uint16_t selector)
{
	idt_set_entry(entry, (uint32_t)intr, selector, INTERRUPT_GATE32);
}

/* Seta uma trap gate na IDT */
void idt_set_trap(int entry, void (*trap)(void), uint16_t selector)
{
	idt_set_entry(entry, (uint32_t)trap, selector, TRAP_GATE32);
}


/* Handler de interrupções */
void intr_handler(intr_frame_t *f)
{
	printf("EAX: 0x%08X ", f->eax);
	printf("EBX: 0x%08X\r\n", f->ebx);
	printf("ECX: 0x%08X ", f->ecx);
	printf("EDX: 0x%08X\r\n", f->edx);
	printf("EBP: 0x%08X ", f->ebp);
	printf("ESI: 0x%08X\r\n", f->esi);
	printf("EDI: 0x%08X\r\n\r\n", f->edi);
	printf("INT: 0x%08X ", f->int_no);
	printf("ERR: 0x%08X\r\n", f->err_code);
	printf("EIP: 0x%08X ", f->eip);
	printf("FLG: 0x%08X\r\n", f->eflags);
	printf("CS:  0x%08X ", f->cs);
	printf("DS:  0x%08X\r\n", f->ds);
	printf("ES:  0x%08X ", f->es);
	printf("FS:  0x%08X\r\n", f->fs);
	printf("GS:  0x%08X\r\n", f->gs);
	__asm__ volatile("cli;hlt");
}

extern void isr_0(void);
extern void isr_1(void);
extern void isr_2(void);
extern void isr_3(void);
extern void isr_4(void);
extern void isr_5(void);
extern void isr_6(void);
extern void isr_7(void);
extern void isr_8(void);
extern void isr_9(void);
extern void isr_10(void);
extern void isr_11(void);
extern void isr_12(void);
extern void isr_13(void);
extern void isr_14(void);
extern void isr_15(void);
extern void isr_16(void);
extern void isr_17(void);
extern void isr_18(void);
extern void isr_19(void);
extern void isr_20(void);
extern void isr_21(void);
extern void isr_22(void);
extern void isr_23(void);
extern void isr_24(void);
extern void isr_25(void);
extern void isr_26(void);
extern void isr_27(void);
extern void isr_28(void);
extern void isr_29(void);
extern void isr_30(void);
extern void isr_31(void);

/* Inicializa IDT basica */
void idt_init(void)
{
	idt_set_trap(0, isr_0, 0x08);
	idt_set_trap(1, isr_1, 0x08);
	idt_set_trap(2, isr_2, 0x08);
	idt_set_trap(3, isr_3, 0x08);
	idt_set_trap(4, isr_4, 0x08);
	idt_set_trap(5, isr_5, 0x08);
	idt_set_trap(6, isr_6, 0x08);
	idt_set_trap(7, isr_7, 0x08);
	idt_set_trap(8, isr_8, 0x08);
	idt_set_trap(9, isr_9, 0x08);
	idt_set_trap(10, isr_10, 0x08);
	idt_set_trap(11, isr_11, 0x08);
	idt_set_trap(12, isr_12, 0x08);
	idt_set_trap(13, isr_13, 0x08);
	idt_set_trap(14, isr_14, 0x08);
	idt_set_trap(15, isr_15, 0x08);
	idt_set_trap(16, isr_16, 0x08);
	idt_set_trap(17, isr_17, 0x08);
	idt_set_trap(18, isr_18, 0x08);
	idt_set_trap(19, isr_19, 0x08);
	idt_set_trap(20, isr_20, 0x08);
	idt_set_trap(21, isr_21, 0x08);
	idt_set_trap(22, isr_22, 0x08);
	idt_set_trap(23, isr_23, 0x08);
	idt_set_trap(24, isr_24, 0x08);
	idt_set_trap(25, isr_25, 0x08);
	idt_set_trap(26, isr_26, 0x08);
	idt_set_trap(27, isr_27, 0x08);
	idt_set_trap(28, isr_28, 0x08);
	idt_set_trap(29, isr_29, 0x08);
	idt_set_trap(30, isr_30, 0x08);
	idt_set_trap(31, isr_31, 0x08);

	idtr.base = &idt[0];
	idtr.limit = IDT_ENTRIES * sizeof(idt_entry_t) - 1;
	__asm__ volatile(
			"LIDT %0;"
			:: "m"(idtr)
	);
}


