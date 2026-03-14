/************************************
 * debug.c                          *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#include <stdarg.h>
#include <stdio.h>
#include <terminal.h>
#include <timer.h>

/* Imprime uma mensagem de debug */
int debugf(const char *fmt, ...)
{
	va_list args;
	char buffer[256];
	int count = 1;
	va_start(args, fmt);
	count = vsnprintf(buffer, sizeof(buffer), fmt, args);

	uint32_t ms = timer_ticks_to_ms(timer_get_ticks());
	uint32_t sec = ms / 1000;
	uint32_t frac = ms % 1000;

	printf("[%7u.%03us] ", sec, frac);

	terminal_putstring(buffer);
	va_end(args);
	return count;
}
