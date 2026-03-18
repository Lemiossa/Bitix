/************************************
 * sched.h                          *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#ifndef SCHED_H
#define SCHED_H
#include <stdint.h>
#include <stdbool.h>

#define PROCESS_STACK_SIZE 4096
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
	uint32_t esp0;
	uint32_t counter;
	uint32_t uptime_ticks;
	uint32_t wake_tick;
	struct process *next;
	uint8_t state;
	uint8_t exit_code;
	uint8_t priority;
} process_t;

extern process_t *current;

uint32_t ms_to_ticks(uint32_t n);
uint32_t ticks_to_ms(uint32_t n);
uint32_t get_ticks(void);
void sched_init(uint32_t n);
void yield(void);
void exit(int code);
void sleep(uint32_t n);
void sleepnb(uint32_t n);
uint32_t spawn(void (*entry)(void), char *name);
void set_priority(uint32_t pid, uint8_t prio);

#endif /* SCHED_H */
