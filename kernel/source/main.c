/************************************
 * main.c                           *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <graphics.h>
#include <vga.h>
#include <terminal.h>
#include <io.h>

#include <boot.h>

#include <gdt.h>
#include <idt.h>
#include <pic.h>
#include <pit.h>

#include <timer.h>

boot_info_t boot_info = {0};

int echo = 1;

#define KBD_MAX_EVENTS 256

#define KEY_CTRL    (1 << 0)
#define KEY_ALT     (1 << 1)
#define KEY_SHIFT   (1 << 2)

typedef struct kbd_event {
	uint8_t sc;
	uint8_t mod;
	int released;
} kbd_event_t;

int count = 0;
int head = 0;
int tail = 0;
kbd_event_t events[KBD_MAX_EVENTS];

/* Lê um evento do teclado */
/* Retorna um número diferente de zero se houver erro */
int kbd_read_event(kbd_event_t *out)
{

}

/* Handler de teclado */
void kbd_handler(void)
{
	uint8_t sc = inb(0x60);
	int released = 0;

	if (sc & 0x80) {
		sc &= ~0x80;
		released = 1;
	}

	pic_eoi(1);
}

/* Inicializa teclado  */
void kbd_init(void)
{
	pic_unmask_irq(1);
}

/* Func principal */
void kernel_main(boot_info_t *bi)
{
	terminal_init();
	terminal_clear(7, 0);
	gdt_init();
	idt_init();
	pic_remap(0x20, 0x28);
	timer_init(100);
	__asm__ volatile("sti");

	boot_info = *bi;

	printf("Kernel iniciado!\r\n");

	for (int count = 0; count < boot_info.e820_entry_count; count++) {
		e820_entry_t entry = boot_info.e820_table[count];
		printf("E820[%d]: 0x%08X-0x%08X:%d\r\n",
				count, (uint32_t)entry.base, (uint32_t)entry.length, entry.type);
	}


	while (1) ;
}
