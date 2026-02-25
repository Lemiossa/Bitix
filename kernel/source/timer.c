/************************************
 * timer.c                          *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#include <stdint.h>
#include <idt.h>
#include <pic.h>
#include <pit.h>

static uint32_t volatile ticks = 0;
static uint16_t freq = 0;

extern void terminal_tick(void);

/* Handler do timer */
void timer_handler(intr_frame_t *f)
{
	(void)f;
	ticks++;
	terminal_tick();
	pic_eoi(0);
}

/* Converte milisegundos para ticks */
uint32_t timer_ms_to_tick(uint32_t ms)
{
	return (ms * freq) / 1000;
}

/* Converte ticks para milisegundos */
uint32_t timer_tick_to_ms(uint32_t ticks)
{
	return ticks * (1000 / freq);
}

/* Espera N ticks do timer */
/* Busy Wait */
void timer_sleep_ticks(uint32_t n)
{
	uint32_t end = ticks + n;
	while (ticks < end);
}

/* Inicializa o timer */
void timer_init(uint16_t frequency)
{
	freq = frequency;
	pit_set(0, PIT_SQUARE_WAVE_GENERATOR, freq);
	pic_unmask_irq(0);
}

