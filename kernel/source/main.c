/************************************
 * main.c                           *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <fpu.h>
#include <cpuid.h>
#include <asm.h>
#include <graphics.h>
#include <vga.h>
#include <terminal.h>
#include <boot.h>
#include <gdt.h>
#include <idt.h>
#include <pic.h>
#include <pit.h>
#include <pci.h>
#include <pmm.h>
#include <vmm.h>
#include <acpi.h>
#include <legacy_timer.h>
#include <timer.h>

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

	graphics_init();

	if (!cpuid_is_available()) {
		printf("CPUID nao esta disponivel\r\n");
		goto halt;
	}
	cpuid_get_features();

	if (!fpu_init()) {
		printf("Falha ao inicializar FPU\r\n");
		goto halt;
	}

	printf("Fornecedor de CPU: %s\r\n", (char *)cpu_vendor);
	if (!acpi_init()) {
		printf("Falha ao iniciar ACPI\r\n");
		goto halt;
	}

	timer_init(5);
	pci_enumerate();

	printf("Procurando dispositivos IDE...\r\n");
	pci_device_t *dev = pci_find(1, 1);
	if (!dev) {
		printf("Nao existe dispositivo IDE\r\n");
	} else {
		printf("Encontrado dispositivo IDE em %hhu:%hhu:%hhu\r\n", dev->bus, dev->dev, dev->func);
		for (int i  = 0; i < 6; i++) {
			printf("BAR[%d] = %08X\r\n", i, dev->bars[i]);
		}
	}

	while (1)
		hlt();

halt:
	printf("Sistema travado. Por favor, reinicie\r\n");
halt_no_msg:
	cli();
	hlt();
}
