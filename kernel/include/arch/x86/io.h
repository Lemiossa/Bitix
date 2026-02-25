/************************************
 * io.h                             *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#ifndef IO_H
#define IO_H
#include <stdint.h>

/* Envia um byte para uma porta IO */
static inline void outb(uint16_t port, uint8_t val)
{
	__asm__ volatile ("OUTB %1, %0" :: "Nd"(port), "a"(val) : "memory");
}

/* Recebe um byte de uma porta IO */
static inline uint8_t inb(uint16_t port)
{
	uint8_t val = 0;
	__asm__ volatile ("INB %1, %b0" : "=Na"(val) : "Nd"(port) : "memory");
	return val;
}

static inline void io_wait(void)
{
	outb(0x80, 0);
}

#endif /* IO_H */
