/************************************
 * sched.c                          *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#include <stdint.h>
#include <stddef.h>
#include <heap.h>
#include <vmm.h>
#include <idt.h>
#include <string.h>
#include <asm.h>
#include <panic.h>
#include <sched.h>
#include <debug.h>

#define PROCESS_STACK_SIZE 16384
#define PROCESS_MAX_NAME 256
uint32_t process_priorities[] =
{
	1,  /* Prioridade 0 */
	4,  /* Prioridade 1 */
	8,  /* Prioridade 2 */
	16, /* Prioridade 3 */
};
#define TOTAL_PRIORITIES (sizeof(process_priorities) / sizeof(process_priorities[0]))

process_t *current = NULL;
uint32_t current_id = 1;

/* Procura o proximo processo pronto */
static inline process_t *sched_get_next_ready(process_t *start)
{
	if (!start)
		return NULL;

	if (start->next == start)
		return start;

	process_t *p = start;
	do
	{
		if (p->state == READY)
			return p;
		p = p->next;
	}
	while (p != start);

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
		p = p->next;
	}
	while (p != current);

	return NULL;
}

/* Retorna o número de quantums de acordo com a prioridade */
static inline uint32_t get_quantums(uint32_t priority)
{
	if (priority >= TOTAL_PRIORITIES)
		return process_priorities[TOTAL_PRIORITIES - 1];
	return process_priorities[priority];
}

/* Escalonador */
void sched(intr_frame_t *f)
{
	if (!current)
		return;

	if (current->next == current)
		return;

	current->uptime_ticks++;

	if (--current->quantum > 0)
		return;

	process_t *p = sched_get_next_ready(current);
	if (!p)
		return;

	current->quantum = get_quantums(current->priority);
	current->esp = (uint32_t)f;
	current->state = READY;

	current = p;
	current->state = RUNNING;

	switch_context((intr_frame_t *)current->esp);
}

/* Inicializa escalonador */
void sched_init(void)
{
	cli();
	debugf("escalonador: Inicializando...\r\n");
	process_t *idle = alloc(sizeof(process_t));
	if (!idle)
		panic("escalonador: Falha ao alocar memoria para o processo idle\r\n");

	memset(idle, 0, sizeof(process_t));

	strncpy(idle->name, "idle", PROCESS_MAX_NAME);
	idle->pid = 0;
	idle->cr3 = (uint32_t)kernel_pd;
	idle->esp = 0;
	idle->state = RUNNING;
	idle->next = idle;
	idle->ppid = 0;
	idle->priority = 0;
	idle->quantum = get_quantums(0);
	idt_set_trap(48, sched, 0x08);

	current = idle;
	sti();
}

/* Cede a execução voluntariamente */
void yield(void)
{
	__asm__ volatile ("INT $48");
		while (1); /* Não deve chegar aqui */
}

/* Sai do processo atual */
void exit(int code)
{
	current->state = DEAD;
	current->quantum = 1;
	current->exit_code = (uint8_t)code;
	yield();
	while (1); /* Não deve executar por muito tempo */
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

	strncpy(proc->name, name, PROCESS_MAX_NAME);
	proc->pid = current_id++;
	proc->ppid = current->pid;
	proc->cr3 = (uint32_t)kernel_pd; /* Por enquanto, usa o mesmo PD do kernel */
	proc->esp = (uint32_t)alloc(PROCESS_STACK_SIZE);

	if (!proc->esp)
	{
		free(proc);
		return 0;
	}

	memset((void *)proc->esp, 0, PROCESS_STACK_SIZE);
	proc->esp += PROCESS_STACK_SIZE - sizeof(intr_frame_t);

	proc->quantum = get_quantums(1);
	proc->priority = 1;
	proc->state = READY;

	intr_frame_t *frame = (intr_frame_t *)proc->esp;

	frame->eflags = 0x202;
	frame->eip = (uint32_t)entry;
	frame->cs = 0x08;

	frame->ds = 0x10;
	frame->es = 0x10;
	frame->fs = 0x10;
	frame->gs = 0x10;

	proc->next = current->next;
	current->next = proc;

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

