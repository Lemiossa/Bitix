/************************************
 * sched.c                          *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#include <stdint.h>
#include <stddef.h>
#include <heap.h>
#include <stdio.h>
#include <vmm.h>
#include <idt.h>
#include <string.h>
#include <asm.h>
#include <panic.h>
#include <sched.h>
#include <debug.h>
#include <sched.h>
#include <fpu.h>
#include <stdbool.h>
#include <terminal.h>
#include <pit.h>
#include <pic.h>

uint32_t process_priorities[] =
{
	16, /* Prioridade 0 */
	8,  /* Prioridade 1 */
	4,  /* Prioridade 2 */
	1,  /* Prioridade 3 */
};
#define TOTAL_PRIORITIES (sizeof(process_priorities) / sizeof(process_priorities[0]))

process_t *current = NULL;
uint32_t current_id = 1;

uint32_t volatile ticks = 0;
static uint16_t freq = 0;

int irq_disable_counter = 0;

/* Desabilita interrupções */
static inline void disable(void)
{
	irq_disable_counter++;
	cli();
}

/* Habilita interrupções */
static inline void enable(void)
{
	if (irq_disable_counter == 0)
		sti();
	else
		irq_disable_counter--;
}

/* Converte n milisegundos para ticks */
uint32_t ms_to_ticks(uint32_t n)
{
	if (n == 0)
		return 0;
	return (n * freq) / 1000;
}

/* Converte n ticks para milisegundos */
uint32_t ticks_to_ms(uint32_t n)
{
	if (n == 0)
		return 0;
	return (n * 1000) / freq;
}

/* Retorna o número de ticks */
uint32_t get_ticks(void)
{
	return ticks;
}

/* Retorna o número de quantums de acordo com a prioridade */
static inline uint32_t get_quantums(uint32_t priority)
{
	if (priority >= TOTAL_PRIORITIES)
		return process_priorities[TOTAL_PRIORITIES - 1];
	return process_priorities[priority];
}

/* Procura o proximo processo pronto */
static inline process_t *sched_get_next_ready(process_t *start)
{
	if (!start)
		return NULL;

	process_t *p = start;
	do
	{
		if (p->state == BLOCKED && ticks >= p->wake_tick)
			p->state = READY;

		if (p->state == READY)
			return p;
	}
	while ((p = p->next) != start);

	return NULL;
}

/* Procura um processo pelo PID */
static inline process_t *sched_get_process(uint32_t pid)
{
	process_t *p = current;
	do
	{
		if (p->pid == pid)
			return p;
	}
	while ((p = p->next) != current);

	return NULL;
}

/* Coloca uma tarefa para "dormir" */
void task_sleep(process_t *p, uint32_t ms)
{
	p->wake_tick = ticks + ms_to_ticks(ms);
	p->state = BLOCKED;
}

/* Escalonador */
void sched(intr_frame_t *f)
{
	if (!current || current->next == current)
		return;

	current->uptime_ticks++;

	if (current->state == RUNNING && current->quantum > 0)
	{
		current->quantum--;
		return;
	}

	process_t *p = sched_get_next_ready(current);
	if (!p)
		return;

	/* Processo atual */
	current->esp0 = (uint32_t)f;
	current->quantum = get_quantums(current->priority);
	if (current->state == RUNNING)
		current->state = READY;

	current = p;

	/* Novo processo */
	current->quantum = get_quantums(current->priority);
	current->state = RUNNING;

	switch_context((intr_frame_t *)current->esp0);
}

/* Handler do IRQ0 */
void time(intr_frame_t *f)
{
	ticks++;
	pic_eoi(0);
	sched(f);
}

/* Inicializa escalonador */
void sched_init(uint32_t n)
{
	cli();
	debugf("escalonador: Inicializando...\r\n");
	process_t *idle = alloc(sizeof(process_t));
	if (!idle)
		panic("escalonador: Falha ao alocar memoria para o processo idle\r\n");

	memset(idle, 0, sizeof(process_t));

	strncpy(idle->name, "idle", PROCESS_MAX_NAME);
	idle->pid = 0;
	idle->ppid = 0;
	idle->cr3 = (uint32_t)kernel_pd;
	idle->esp = 0;
	idle->esp0 = 0;
	idle->state = RUNNING;
	idle->next = idle;
	idle->priority = 3;
	idle->quantum = get_quantums(idle->priority);
	idt_set_intr(50, sched, 0x08);

	freq = n;
	ticks = 0;
	pit_set(0, PIT_SQUARE_WAVE_GENERATOR, n);
	idt_set_trap(0x20, time, 0x08);
	pic_unmask_irq(0);

	current = idle;
	sti();
}

/* Cede a execução voluntariamente */
void yield(void)
{
	__asm__ volatile ("INT $50");
	while (1); /* Não deve chegar aqui */
}

/* Sai do processo atual */
void exit(int code)
{
	if (!current || current->pid == 0)
		panic("escalonador: Tentativa de sair no idle\r\n");

	disable();
	current->state = DEAD;
	current->exit_code = (uint8_t)code;
	enable();
	yield();
}

/* Espera N ms em uma tarefa */
void sleep(uint32_t n)
{
	if (!current || current->pid == 0)
		panic("escalonador: Tentativa de esperar no idle\r\n");

	uint32_t end = ticks + ms_to_ticks(n);

	disable();
	current->wake_tick = end;
	current->state = BLOCKED;
	enable();

	yield();

	while ((ticks - end) > 0);
}

/* Cria um novo processo */
/* Retorna o PID dele, 0 se houver erro */
uint32_t spawn(void (*entry)(void), char *name)
{
	if (!current)
		return 0;

	process_t *proc = alloc(sizeof(process_t));
	if (!proc)
		return 0;

	memset(proc, 0, sizeof(process_t));
	cli();
	strncpy(proc->name, name, PROCESS_MAX_NAME);
	proc->pid = current_id++;
	proc->ppid = current->pid;
	proc->cr3 = (uint32_t)kernel_pd; /* Por enquanto, usa o mesmo PD do kernel */
	proc->esp = 0;
	proc->esp0 = (uint32_t)alloc(PROCESS_STACK_SIZE);

	if (!proc->esp0)
	{
		free(proc);
		sti();
		return 0;
	}

	memset((void *)proc->esp0, 0, PROCESS_STACK_SIZE);
	proc->esp0 += PROCESS_STACK_SIZE - sizeof(intr_frame_t);

	proc->priority = 2;
	proc->quantum = get_quantums(proc->priority);
	proc->state = READY;

	intr_frame_t *frame = (intr_frame_t *)proc->esp0;

	frame->eflags = 0x202;
	frame->eip = (uint32_t)entry;
	frame->cs = 0x08;

	frame->ds = 0x10;
	frame->es = 0x10;
	frame->fs = 0x10;
	frame->gs = 0x10;

	proc->next = current->next;
	current->next = proc;
	sti();

	return proc->pid;
}

/* Muda a prioridade de um processo */
void set_priority(uint32_t pid, uint32_t priority)
{
	if (priority >= TOTAL_PRIORITIES)
		return;

	process_t *p = sched_get_process(pid);
	if (!p)
		return;

	p->priority = priority;
}

