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
#include <io.h>

#include <boot.h>

#include <gdt.h>
#include <idt.h>
#include <pic.h>
#include <pit.h>

#include <pmm.h>
#include <vmm.h>

#include <acpi.h>

boot_info_t boot_info = {0};

volatile uint32_t pit0_timer_ticks = 0;
void pit0_timer_handler(void)
{
	pit0_timer_ticks++;
	pic_eoi(0);
}

/* Calibra o LAPIC timer */
/* Retorna o número de ticks por segundo */
uint32_t lapic_timer_calibrate(void)
{
	acpi_lapic_write(0x320, 0x20); /* Modo Oneshot */
	acpi_lapic_write(0x3E0, 0xB); /* Divide por 1 */
	acpi_lapic_write(0x380, 0xFFFFFFFF); /* O Contador pro maximo */
	acpi_lapic_write(0x320, 0x10020);

	pit_set(0, PIT_SQUARE_WAVE_GENERATOR, 100);
	pic_set_irq_handler(0, pit0_timer_handler);
	pic_unmask_irq(0);
	__asm__ volatile("sti");
	uint32_t start = pit0_timer_ticks;
	while ((pit0_timer_ticks - start) < 10);
	uint32_t current = acpi_lapic_read(0x390);
	uint32_t elapsed = 0xFFFFFFFF - current;
	return (elapsed / 10) * 100;
}

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

	printf("\033[32mKernel iniciado!\033[0m\r\n");
	printf("Modo de video: \033[32m%d\033[0mx\033[33m%d\033[0mx\033[34m%d\033[0m\r\n", boot_info.graphics.width, boot_info.graphics.height,
			boot_info.graphics.bpp);

	printf("Ponteiro para e820_table: 0x%08X\r\n", boot_info.e820_table);
	for (int count = 0; count < boot_info.e820_entry_count; count++) {
		e820_entry_t entry = boot_info.e820_table[count];
		printf("E820[%d]: 0x%08X-0x%08X:%d\r\n",
				count, (uint32_t)entry.base, (uint32_t)entry.length, entry.type);
	}

	printf("Fornecedor de CPU: %s\r\n", (char *)cpu_vendor);
	if (!acpi_init()) {
		printf("Falha ao iniciar ACPI\r\n");
		goto halt;
	}

	acpi_lapic_write(0xF0, acpi_lapic_read(0xF0) | 0x1FF);
	uint32_t lapic_timer_frec = lapic_timer_calibrate();
	printf("Frequencia do lapic timer: %u\r\n", lapic_timer_frec);

halt:
	printf("Sistema travado. Por favor, reinicie\r\n");
halt_no_msg:
	while (1);
}
