/************************************
 * pit.h                            *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#ifndef PIT_H
#define PIT_H
#include <stdint.h>

#define PIT_INTERRUPT_ON_TERMINAL_COUNT 0
#define PIT_HARDWARE_RETRIGGERABLE_ONE_SHOT 1
#define PIT_RATE_GENERATOR 2
#define PIT_SQUARE_WAVE_GENERATOR 3
#define PIT_SOFTWARE_TRIGGERED_STROBE 4
#define PIT_HARDWARE_TRIGGERED_STROBE 5

void pit_set(int channel, uint8_t op_mode, uint32_t freq);

#endif /* PIT_H */
