/************************************
 * string.c                         *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#include <stdint.h>
#include <stddef.h>

/* Copia N bytes de SRC para DEST */
/* Retorna DEST */
void *memcpy(void *dest, const void *src, size_t n)
{
	uint8_t *d = (uint8_t *)dest;
	uint8_t *s = (uint8_t *)src;

	for (size_t i = 0; i < n; i++) {
		d[i] = s[i];
	}

	return d;
}

/* Seta N bytes em DEST com valor B */
void memset(void *dest, int b, size_t n)
{
	uint8_t *d = (uint8_t *)dest;

	for (size_t i = 0; i < n; i++) {
		d[i] = (uint8_t)b;
	}
}
