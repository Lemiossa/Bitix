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

void prog1(void)
{
	while (1)
	{
		printf("%8s %8s %8s %8s %s\r\n\r\n", "pid", "ppid", "state", "uptime", "name");
		process_t *p = current;
		do
		{
			char *str = "unknown";

			if (p->state == READY)
				str = "ready";
			else if (p->state == RUNNING)
				str = "running";
			else if (p->state == BLOCKED)
				str = "blocked";
			else if (p->state == DEAD)
				str = "dead";

			printf("%8u %8u %8s %8u %8u %s\r\n", p->pid, p->ppid, str, ticks_to_ms(p->uptime_ticks), p->name);
			p = p->next;

		}
		while (p != current);
		sleep(1000);
	}
	exit(0);
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
	sched_init(100);
	terminal_init();
	terminal_clear(TERMINAL_DEFAULT_FG_COLOR, TERMINAL_DEFAULT_BG_COLOR);
	cpuid_init();
	fpu_init();
	acpi_init();
	pci_enumerate();
	ata_detect();

	printf("Spawnando o prog!\r\n");

	spawn(prog1, "prog1");

	while (1)
	{
		hlt();
	}
}
