/************************************
 * idt.c                            *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#include <asm.h>
#include <idt.h>
#include <stdint.h>
#include <stdio.h>
#include <terminal.h>
#include <panic.h>

#define INTERRUPT_GATE32 0x8E
#define TRAP_GATE32 0x8F

idtr_t idtr;
idt_entry_t idt[IDT_ENTRIES];

void (*intrs[IDT_ENTRIES])(intr_frame_t *f) = {0};

extern void (*isrs[256])(void);

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
void idt_set_intr(int entry, void (*intr)(intr_frame_t *f), uint16_t selector)
{
	if (entry >= IDT_ENTRIES)
		return;

	idt_set_entry(entry, (uint32_t)isrs[entry], selector, INTERRUPT_GATE32);
	intrs[entry] = intr;
}

/* Seta uma trap gate na IDT */
void idt_set_trap(int entry, void (*trap)(intr_frame_t *f), uint16_t selector)
{
	if (entry >= IDT_ENTRIES)
		return;

	idt_set_entry(entry, (uint32_t)isrs[entry], selector, TRAP_GATE32);
	intrs[entry] = trap;
}

/* Handler de interrupções */
void intr_handler(intr_frame_t *f)
{
	if (intrs[f->int_no])
	{
		intrs[f->int_no](f);
		return;
	}

	printf("Interrupcao %u\r\n", f->int_no);
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
	panic("Interrupcoes sem handler definido sao tratadas como excecao\r\n");
	cli();
	hlt();
}

/* Inicializa IDT basica */
void idt_init(void)
{
	for (int i = 0; i < 32; i++)
	{
		idt_set_entry(i, (uint32_t)isrs[i], 0x08, INTERRUPT_GATE32);
	}

	for (int i = 32; i < 256; i++)
	{
		idt_set_entry(i, (uint32_t)isrs[i], 0x08, TRAP_GATE32);
	}

	idtr.base = &idt[0];
	idtr.limit = IDT_ENTRIES * sizeof(idt_entry_t) - 1;
	__asm__ volatile("LIDT %0;" ::"m"(idtr));
}
