/************************************
 * main.c                           *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
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
#include <terminal.h>
#include <vga.h>
#include <vmm.h>
#include <panic.h>
#include <sched.h>
#include <debug.h>
#include <vfs.h>
#include <acpi.h>

boot_info_t boot_info = {0};

/* Func principal */
void kernel_main(boot_info_t *bi)
{
	boot_info = *bi;
	gdt_init();
	idt_init();
	pic_remap(0x20, 0x28);
	pmm_init();
	vmm_init();
	graphics_init();
	heap_init();
	terminal_init();
	terminal_clear(TERMINAL_DEFAULT_FG_COLOR, TERMINAL_DEFAULT_BG_COLOR);
	debugf("Terminal inicializado: %dx%d\r\n", terminal_get_width(), terminal_get_height());
	sched_init(100);
	debugf("Modo de video: %s\r\n", boot_info.graphics.mode ? "VESA" : "VGA/Outro");

	if (boot_info.graphics.mode == 1)
	{
		debugf("\tResolucao: %dx%d\r\n", boot_info.graphics.width, boot_info.graphics.height);
		debugf("\tPitch: %d\r\n", boot_info.graphics.pitch);
		debugf("\tBPP: %d\r\n", boot_info.graphics.bpp);
		debugf("\tFramebuffer: 0x%08X\r\n", boot_info.graphics.framebuffer);
	}
	
	cpuid_init();
	fpu_init();
	acpi_init();
	pci_enumerate();
	ata_detect();
	
	while (1)
	{
		hlt();
	}
}
