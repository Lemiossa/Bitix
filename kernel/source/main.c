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
#include <font8x8.h>

boot_info_t boot_info = {0};

/* Lista um diretorio */
void list(const char *path)
{
	dir_t *d = opendir(path);
	if (!d)
		return;

	while (1)
	{
		dirent_t dirent;
		if (!readdir(d, &dirent))
			break;

		printf("%s/%s\r\n", path, dirent.name);
	}
	closedir(d);
}

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
	sched_init(100);
	cpuid_init();
	fpu_init();
	acpi_init();
	pci_enumerate();
	ata_detect();

	list("C:/");
	list("D:/");
	list("E:/");

	while (1)
	{
		hlt();
	}
}
