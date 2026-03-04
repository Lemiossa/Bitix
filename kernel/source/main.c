/************************************
 * main.c                           *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <cpuid.h>

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

#include <timer.h>
#include <kbd.h>

boot_info_t boot_info = {0};
char *cpu_vendor = "            ";
uint32_t cpu_features_ebx = 0;
uint32_t cpu_features_ecx = 0;
uint32_t cpu_features_edx = 0;

/* Pega todas as features da CPU e coloca na variavel global CPU features ebx, ecx e edx*/
void cpuid_get_features(void)
{
	uint32_t ebx = 0, ecx = 0, edx = 0;
	cpuid(0, NULL, &ebx, &ecx, &edx);

	((uint32_t *)cpu_vendor)[0] = ebx;
	((uint32_t *)cpu_vendor)[1] = edx;
	((uint32_t *)cpu_vendor)[2] = ecx;
	cpuid(1, NULL, &cpu_features_ebx, &cpu_features_ecx, &cpu_features_edx);
}

/* Func principal */
void kernel_main(boot_info_t *bi)
{
	boot_info = *bi;
	gdt_init();
	idt_init();
	if (!pmm_init())
		goto halt_no_msg;

	graphics_init();
	terminal_init();
	terminal_clear(TERMINAL_DEFAULT_FG_COLOR, TERMINAL_DEFAULT_BG_COLOR);

	pic_remap(0x20, 0x28);
	timer_init(100);
	kbd_init();
	__asm__ volatile("sti");

	printf("\033[32mKernel iniciado!\033[0m\r\n");
	printf("Modo de video: \033[32m%d\033[0mx\033[33m%d\033[0mx\033[34m%d\033[0m\r\n", boot_info.graphics.width, boot_info.graphics.height,
			boot_info.graphics.bpp);

	printf("Ponteiro para e820_table: 0x%08X\r\n", boot_info.e820_table);
	for (int count = 0; count < boot_info.e820_entry_count; count++) {
		e820_entry_t entry = boot_info.e820_table[count];
		printf("E820[%d]: 0x%08X-0x%08X:%d\r\n",
				count, (uint32_t)entry.base, (uint32_t)entry.length, entry.type);
	}


	if (!cpuid_is_available()) {
		printf("\033[31mERRO: CPUID nao esta disponivel\r\n");
		goto halt;
	}
	cpuid_get_features();

	printf("Fornecedor de CPU: %s\r\n", (char *)cpu_vendor);
halt:
	printf("Sistema travado. Por favor, reinicie\r\n");
halt_no_msg:
	while (1);
}
