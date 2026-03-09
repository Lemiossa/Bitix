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
#include <string.h>
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
#include <legacy_timer.h>
#include <timer.h>

boot_info_t boot_info = {0};

#define MAX_PROCESSES 256

#define PROC_QUANTUM 4

#define PROC_READY 0
#define PROC_RUNNING 1
#define PROC_BLOCKED 2

typedef struct process {
	uint32_t id;
	uint32_t cr3;
	uint32_t esp;
	uint32_t quantum;
	uint8_t state;
} process_t;

process_t processes[MAX_PROCESSES];

uint32_t current_id = 0;
uint32_t process_count = 0;

/* Cria um novo processo */
void process_create(void (*entry)(void))
{
	if (process_count > MAX_PROCESSES)
		return;

	processes[process_count].id = process_count;
	processes[process_count].cr3 = (uint32_t)kernel_pd;
	processes[process_count].quantum = 0;
	processes[process_count].state = PROC_READY;

	uint32_t phys = (uint32_t)pmm_alloc_page();
	uint32_t stack = vmm_get_free_virt_user();
	vmm_map(phys, stack, PAGE_PRESENT | PAGE_WRITE);
	uint32_t esp = stack + PAGE_SIZE;
	processes[process_count].esp = esp;

	intr_frame_t *ctx = (intr_frame_t *)(esp - sizeof(*ctx));
	processes[process_count].esp = (uint32_t)ctx;

	memset(ctx, 0, sizeof(*ctx));

	ctx->cs = 0x08;
	ctx->ds = 0x10;
	ctx->es = 0x10;
	ctx->fs = 0x10;
	ctx->gs = 0x10;

	ctx->eflags = 0x202;
	ctx->eip = (uint32_t)entry;

	process_count++;
}

/* Inicializa escalonador */
void scheduler_init(void)
{
	processes[0].id = 0;
	processes[0].cr3 = (uint32_t)kernel_pd;
	processes[0].state = PROC_RUNNING;
	process_count = 1;
	current_id = 0;
}

/* Escalonador */
void scheduler(void)
{
	if (process_count <= 1)
		return;

	processes[current_id].quantum++;
	if (processes[current_id].quantum <=  PROC_QUANTUM)
		return;

	processes[current_id].quantum = 0;
	processes[current_id].esp = (uint32_t)last_frame;
	processes[current_id].state = PROC_READY;

	uint32_t next = (current_id + 1) % process_count;
	uint32_t checked = 0;
	while (processes[next].state != PROC_READY) {
		next = (next + 1) % process_count;
		checked++;
		if (checked >= process_count) {
			next = 0;
			break;
		}
	}
	current_id = next;

	processes[current_id].state = PROC_RUNNING;

	set_cr3(processes[current_id].cr3);
	switch_context((intr_frame_t *)processes[current_id].esp);
}

void proc1(void)
{
	while (1) {
		printf("1");
		timer_wait(1000);
	}
}

void proc2(void)
{
	while (1) {
		printf("2");
		timer_wait(1000);
	}
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



	while (1)
		hlt();

halt:
	printf("Sistema travado. Por favor, reinicie\r\n");
halt_no_msg:
	cli();
	hlt();
}
