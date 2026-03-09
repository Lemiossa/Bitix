/************************************
 * legacy_timer.c                   *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#include <stdint.h>
#include <pit.h>
#include <asm.h>
#include <idt.h>
#include <pic.h>

uint32_t volatile legacy_timer_ticks = 0;
uint16_t legacy_timer_frequency = 0;

/* Timer antigo: handler */
void legacy_timer_handler(void)
{
	legacy_timer_ticks++;
	pic_eoi(0);
}

/* Timer antigo: Inicia */
void legacy_timer_init(uint16_t freq)
{
	cli();
	legacy_timer_frequency = freq;
	legacy_timer_ticks = 0;
	pit_set(0, PIT_SQUARE_WAVE_GENERATOR, freq);
	idt_set_trap(0x20, legacy_timer_handler, 0x08);
	pic_unmask_irq(0);
	sti();
}

/* Timer antigo: Desativa */
void legacy_timer_disable(void)
{
	pic_mask_irq(0);
}

/* Timer antigo: busy wait */
void legacy_timer_wait(uint32_t ms)
{
	uint32_t end = legacy_timer_ticks + ((ms * legacy_timer_frequency) / 1000);
	while (legacy_timer_ticks < end);
}

