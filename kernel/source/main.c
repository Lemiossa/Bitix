/************************************
 * main.c                           *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#include <acpi.h>
#include <asm.h>
#include <ata.h>
#include <boot.h>
#include <cpuid.h>
#include <fat.h>
#include <fpu.h>
#include <gdt.h>
#include <graphics.h>
#include <heap.h>
#include <idt.h>
#include <pci.h>
#include <pic.h>
#include <pit.h>
#include <pmm.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <terminal.h>
#include <timer.h>
#include <vga.h>
#include <vmm.h>
#include <panic.h>
#include <sched.h>

boot_info_t boot_info = {0};

/* Func principal */
void kernel_main(boot_info_t *bi)
{
	boot_info = *bi;
	gdt_init();
	idt_init();
	pic_remap(0x20, 0x28);
	terminal_init();
	pmm_init();
	vmm_init();
	heap_init();
	graphics_init();
	terminal_clear(TERMINAL_DEFAULT_FG_COLOR, TERMINAL_DEFAULT_BG_COLOR);
	cpuid_init();
	fpu_init();
	acpi_init();
	pci_enumerate();
	ata_detect();
	timer_init(100);
	sched_init();

	while (1)
	{
		hlt();
	}
}
