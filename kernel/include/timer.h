/************************************
 * timer.h                          *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#ifndef TIMER_H
#define TIMER_H
#include <stdint.h>

uint32_t timer_ms_to_ticks(uint32_t n);
uint32_t timer_ticks_to_ms(uint32_t n);
uint32_t timer_get_ticks(void);
void timer_init(uint16_t freq);
void timer_disable(void);
void timer_wait(uint32_t ms);

#endif /* TIMER_H */
