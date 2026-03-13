/************************************
 * timer.c                          *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#include <asm.h>
#include <idt.h>
#include <pic.h>
#include <pit.h>
#include <stdint.h>
#include <terminal.h>

uint32_t volatile timer_ticks = 0;
uint16_t timer_freq = 0;

extern void sched(intr_frame_t *f);

/* Handler handler */
void timer_handler(intr_frame_t *f)
{
	timer_ticks++;
	pic_eoi(0);
	sched(f);
}

/* Inicia o timer com N frequencia */
void timer_init(uint16_t n)
{
	cli();
	timer_freq = n;
	timer_ticks = 0;
	pit_set(0, PIT_SQUARE_WAVE_GENERATOR, n);
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
	uint32_t ticks = (n * timer_freq) / 1000;
	if (!ticks)
		ticks = 1;

	uint32_t end = timer_ticks + ticks;
	while (timer_ticks < end)
		;
}
