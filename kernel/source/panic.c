/************************************
 * panic.c                          *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#include <stdio.h>
#include <terminal.h>
#include <asm.h>

/* Imprime uma mensagem de panico e trava */
void panic(const char *fmt, ...)
{
	va_list args;
	char buffer[256];
	va_start(args, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, args);
	printf("Panico: ");
	terminal_putstring(buffer);
	va_end(args);
	cli();
	hlt();
}
