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

boot_info_t boot_info = {0};


/* Func principal */
void kernel_main(boot_info_t *bi)
{
	boot_info = *bi;
	gdt_init();
	idt_init();
	pic_remap(0x20, 0x28);
	terminal_init();
	terminal_clear(TERMINAL_DEFAULT_FG_COLOR, TERMINAL_DEFAULT_BG_COLOR);
	if (!pmm_init())
		goto halt_no_msg;
	if (!vmm_init())
		goto halt_no_msg;
	heap_init();

	graphics_init();

	if (!cpuid_is_available())
	{
		printf("CPUID nao esta disponivel\r\n");
		goto halt;
	}
	cpuid_get_features();

	if (!fpu_init())
	{
		printf("Falha ao inicializar FPU\r\n");
		goto halt;
	}

	printf("Fornecedor de CPU: %s\r\n", (char *)cpu_vendor);
	if (!acpi_init())
	{
		printf("Falha ao iniciar ACPI\r\n");
		goto halt;
	}

	timer_init(4);
	pci_enumerate();
	ata_detect();

	for (int i = 0; i < ata_disk_count; i++)
	{
		printf("Disco %d: %s | Serial: %s | %u b\r\n", i, ata_disks[i].model,
			   ata_disks[i].serial, ata_disks[i].total_sectors * 512);
	}

	while (1)
		hlt();

halt:
	printf("Sistema travado. Por favor, reinicie\r\n");
halt_no_msg:
	cli();
	hlt();
}
