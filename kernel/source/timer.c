/************************************
 * timer.c                          *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#include <stdint.h>
#include <acpi.h>
#include <legacy_timer.h>
#include <idt.h>
#include <asm.h>

uint32_t timer_ms_per_tick = 0;
uint32_t volatile timer_ticks = 0;

/* Handler do timer */
void timer_handler(void)
{
	timer_ticks++;
	acpi_lapic_write(0xB0, 0); /* EOI */
}

/* Busy wait */
void timer_wait(uint32_t ms)
{
	uint32_t ticks = ms / timer_ms_per_tick;
	if (ticks == 0) ticks = 1;
	uint32_t end = timer_ticks + ticks;
	while (timer_ticks < end);
}

/* Inicializa o APIC timer com n MS por tick */
void timer_init(uint32_t ms)
{
	if (ms < 1)
		return;

	acpi_lapic_write(0x3E0, 0xB);
	acpi_lapic_write(0x380, 0xFFFFFFFF);

	legacy_timer_init(1000); /* 1 MS por tick */
	legacy_timer_wait(ms);
	legacy_timer_disable();
	acpi_lapic_write(0x320, 0x10020);
	uint32_t elapsed = 0xFFFFFFFF - acpi_lapic_read(0x390);
	idt_set_trap(0x20, timer_handler, 0x08);

	acpi_lapic_write(0x320, 0x20 | (1 << 17));
	acpi_lapic_write(0x3E0, 0xB);
	acpi_lapic_write(0x380, elapsed);
	timer_ms_per_tick = ms;
	sti();
}

