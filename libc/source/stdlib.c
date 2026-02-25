/************************************
 * stdlib.c                         *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#include <stdint.h>

/* Retorna o valor absoluto de um número */
/* Exemplo: abs(-5) = 5 */
int abs(int x)
{
	if (x == INT32_MIN)
		return INT32_MAX;
	return x < 0 ? -x : x;
}
