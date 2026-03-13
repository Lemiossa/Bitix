/************************************
 * panic.c                          *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#include <stdio.h>
#include <terminal.h>
#include <asm.h>

/* Imprime uma mensagem de panico e trava */
void panic(const char *msg)
{
	printf("Panico: %s\r\n", msg);
	cli();
	hlt();
}
