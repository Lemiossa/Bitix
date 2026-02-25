/************************************
 * pit.c                            *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#include <stdint.h>
#include <io.h>

#define PIT_BASE 0x40
#define PIT_CMD PIT_BASE + 3
#define PIT_BASE_FREQ 1193180

/* Modifica um PIT */
void pit_set(int channel, uint8_t op_mode, uint32_t freq)
{
	if (channel > 2 || freq == 0)
		return;

	uint8_t value = 0b00110000;
	value |= channel << 6;
	value |= ((op_mode & 0b111) << 1);

	uint16_t divisor = PIT_BASE_FREQ / freq;

	outb(PIT_CMD, value);
	outb(PIT_BASE+channel, (divisor & 0xFF));
	outb(PIT_BASE+channel, ((divisor >> 8) & 0xFF));
}
