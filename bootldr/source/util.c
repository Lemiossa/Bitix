/************************************
 * util.c                           *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#include "string.h"

/* Converte um caractere para maiusculo */
char to_upper(char c)
{
	if (c >= 'a' && c <= 'z')
		return c - ('a' - 'A');
	return c;
}

/* Converte um caractere para minusculo */
char to_lower(char c)
{
	if (c >= 'A' && c <= 'Z')
		return c + ('a' - 'A');
	return c;
}

/* Separa uma path em **pathparts */
/* Retorna o número de partes de uma path */
int get_path_parts(char *path, char **parts, int max)
{
	if (max == 0 || !path)
		return 0;

	int count = 0;
	char *part = strtok(path, "/\\");
	parts[count++] =  part;

	while (part != NULL && count < max) {
		part = strtok(NULL, "/\\");
		if (part)
			parts[count++] =  part;
	}

	return count;
}

