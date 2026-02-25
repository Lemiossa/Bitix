/************************************
 * timer.h                          *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#ifndef TIMER_H
#define TIMER_H
#include <stdint.h>

uint32_t timer_ms_to_tick(uint32_t ms);
uint32_t timer_tick_to_ms(uint32_t ticks);
void timer_sleep_ticks(uint32_t n);
void timer_init(uint16_t frequency);

#endif /* TIMER_H */
