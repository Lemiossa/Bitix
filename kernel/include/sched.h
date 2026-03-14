/************************************
 * sched.h                          *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#ifndef SCHED_H
#define SCHED_H
#include <stdint.h>

#define PROCESS_STACK_SIZE 16384
#define PROCESS_MAX_NAME 256

#define READY 0
#define RUNNING 1
#define BLOCKED 2
#define DEAD 3

typedef struct process
{
	char name[PROCESS_MAX_NAME];
	uint32_t pid;
	uint32_t ppid;
	uint32_t cr3;
	uint32_t esp;
	uint32_t priority;
	uint32_t quantum;
	uint32_t uptime_ticks;
	struct process *next;
	uint8_t state;
	uint8_t exit_code;
} process_t;

extern process_t *current;

void sched_init(void);
void yield(void);
void exit(int code);
uint32_t spawn(void (*entry)(void), char *name);
void set_priority(uint32_t pid, uint32_t priority);

#endif /* SCHED_H */
