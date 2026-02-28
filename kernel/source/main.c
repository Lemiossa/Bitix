/************************************
 * main.c                           *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#include <stdint.h>
#include <stdbool.h>
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

#include <pmm.h>

#include <timer.h>
#include <kbd.h>

boot_info_t boot_info = {0};

/* Func principal */
void kernel_main(boot_info_t *bi)
{
	boot_info = *bi;
	terminal_init();
	terminal_clear(7, 0);
	gdt_init();
	idt_init();
	pic_remap(0x20, 0x28);
	pmm_init();
	timer_init(100);
	kbd_init();
	__asm__ volatile("sti");

	printf("Kernel iniciado!\r\n");

	for (int count = 0; count < boot_info.e820_entry_count; count++) {
		e820_entry_t entry = boot_info.e820_table[count];
		printf("E820[%d]: 0x%08X-0x%08X:%d\r\n",
				count, (uint32_t)entry.base, (uint32_t)entry.length, entry.type);
	}

	printf("\033[0;40;40;40P");   // 0 -> 28 28 28
	printf("\033[1;234;105;98P");
	printf("\033[2;169;182;101P");
	printf("\033[3;216;166;87P");
	printf("\033[4;125;174;163P");
	printf("\033[5;211;134;155P");
	printf("\033[6;137;180;130P");
	printf("\033[7;212;190;152P");

	printf("\033[8;60;56;54P");
	printf("\033[9;234;105;98P");
	printf("\033[10;169;182;101P");
	printf("\033[11;216;166;87P");
	printf("\033[12;125;174;163P");
	printf("\033[13;211;134;155P");
	printf("\033[14;137;180;130P");
	printf("\033[15;251;241;199P");

	terminal_clear(TERMINAL_DEFAULT_FG_COLOR, TERMINAL_DEFAULT_BG_COLOR);

	printf("\033[0m=== MATRIZ FG 30-37 x BG 40-47 ===\r\n\r\n");

	for (int bg = 40; bg <= 47; bg++) {
		for (int fg = 30; fg <= 37; fg++) {
			printf("\033[%d;%dm %02d/%02d \033[0m", fg, bg, fg, bg);
		}
		printf("\r\n");
	}

	printf("\r\n=== MATRIZ FG 90-97 x BG 100-107 ===\r\n\r\n");

	for (int bg = 100; bg <= 107; bg++) {
		for (int fg = 90; fg <= 97; fg++) {
			printf("\033[%d;%dm %02d/%02d \033[0m", fg, bg, fg, bg);
		}
		printf("\r\n");
	}

	while (1);
}
