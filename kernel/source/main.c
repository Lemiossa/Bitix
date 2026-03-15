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
#include <terminal.h>
#include <timer.h>
#include <vga.h>
#include <vmm.h>
#include <panic.h>
#include <sched.h>
#include <debug.h>
#include <string.h>
#include <vfs.h>

boot_info_t boot_info = {0};

/* Lista um diretório */
void list_dir(const char *path)
{
	vfs_dir_t *d = vfs_opendir(path);
	if (!d)
		return;

	vfs_dirent_t dirent = {0};
	while (1)
	{
		if (!vfs_readdir(d, &dirent))
			break;

		printf("%s\r\n", dirent.name);
	}

	vfs_closedir(d);
}

/* Lista um diretório recursivamente */
void list_dir_rec(const char *path, int depth)
{
	vfs_dir_t *d = vfs_opendir(path);
	if (!d)
		return;

	vfs_dirent_t dirent = {0};
	while (1)
	{
		if (!vfs_readdir(d, &dirent))
			break;

		for (int i = 0; i < depth; i++)
			printf("\t");

		printf("%s\r\n", dirent.name);

		if (strcmp(dirent.name, ".") == 0 || strcmp(dirent.name, "..") == 0)
			continue;

		char *new_path = alloc(1024);
		if (!new_path)
			continue;
		strncpy(new_path, path, 1024);
		strcat(new_path, dirent.name);
		list_dir_rec(new_path, depth + 1);
	}

	vfs_closedir(d);
}

/* Func principal */
void kernel_main(boot_info_t *bi)
{
	boot_info = *bi;
	gdt_init();
	idt_init();
	pic_remap(0x20, 0x28);
	timer_init(100);
	pmm_init();
	vmm_init();
	graphics_init();
	heap_init();
	terminal_init();
	terminal_clear(TERMINAL_DEFAULT_FG_COLOR, TERMINAL_DEFAULT_BG_COLOR);
	cpuid_init();
	fpu_init();
	acpi_init();
	pci_enumerate();
	ata_detect();
	sched_init();
	fat_registry(0);

	list_dir_rec("C:/", 0);

	while (1)
	{
		hlt();
	}
}
