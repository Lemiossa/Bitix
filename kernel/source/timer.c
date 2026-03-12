/************************************
 * timer.c                   *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#include <asm.h>
#include <idt.h>
#include <pic.h>
#include <pit.h>
#include <stdint.h>

uint32_t volatile timer_ticks = 0;
uint16_t timer_ms_per_tick = 0;

/* Handler handler */
void timer_handler(void)
{
	timer_ticks++;
	pic_eoi(0);
}

/* Inicia o timer com N ms por tick*/
void timer_init(uint16_t n)
{
	cli();
	uint16_t freq = 1000 / n;
	timer_ms_per_tick = 1000 / freq;
	timer_ticks = 0;
	pit_set(0, PIT_SQUARE_WAVE_GENERATOR, freq);
	idt_set_trap(0x20, timer_handler, 0x08);
	pic_unmask_irq(0);
	sti();
}

/* Desativa o timer */
void timer_disable(void)
{
	pic_mask_irq(0);
}

/* Espera N milisegundos usando o timer */
void timer_wait(uint32_t n)
{
	uint32_t ticks = n / timer_ms_per_tick;
	if (!ticks)
		ticks = 1;

	uint32_t end = timer_ticks + n;
	while (timer_ticks < end)
		;
}
