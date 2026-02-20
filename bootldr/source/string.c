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

/* Compara 2 strings e retorna a diferença entre elas */
size_t strcmp(const char *s1, const char *s2)
{
	while (*s1 && (*s1 == *s2)) {
		s1++;
		s2++;
	}

	return (unsigned char)*s1 - (unsigned char)*s2;
}

/* Compara N caracteres de 2 strings e retorna a diferença entre elas */
size_t strncmp(const char *s1, const char *s2, size_t n)
{
	for (size_t i = 0; i < n && (*s1 == *s2); i++) {
		s1++;
		s2++;
	}

	return (unsigned char)*s1 - (unsigned char)*s2;
}

/* Retorna o tamanho de uma string */
size_t strlen(const char *s)
{
	size_t i = 0;
	while (*s) {
		s++;
		i++;
	}

	return i;
}

/* Retorna um ponteiro para a primeira ocorrencia de um caractere em uma string */
/* Retorna nulo quando não encontra nada */
char *strchr(const char *s, char ch)
{
	char *p = (char *)s;
	while (*p) {
		if (*p == ch) {
			return p;
		}
		p++;
	}
	return NULL; /* Não achou */
}

/* Separa uma string usando delimitadores */
char *strtok(char *str, const char *delim)
{
	static char *saved;
	if (str)
		saved = str;

	char *p = saved;

	while (*p && strchr(delim, *p))
		p++;

	char *start = p;

	while (*p && !strchr(delim, *p))
		p++;

	if (*p) {
		*p = 0;
		saved = p + 1;
	} else {
		saved = NULL;
	}

	return start;
}
