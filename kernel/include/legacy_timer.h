/************************************
 * legacy_timer.h                   *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#ifndef LEGACY_TIMER_H
#define LEGACY_TIMER_H
#include <stdint.h>

void legacy_timer_init(uint16_t freq);
void legacy_timer_disable(void);
void legacy_timer_wait(uint32_t ms);

#endif /* LEGACY_TIMER_H */
