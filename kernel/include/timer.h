/************************************
 * timer.h                          *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#ifndef TIMER_H
#define TIMER_H
#include <stdint.h>

void timer_init(uint16_t freq);
void timer_disable(void);
void timer_wait(uint32_t ms);

#endif /* TIMER_H */
