/************************************
 * ctype.h                          *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#ifndef CTYPE_H
#define CTYPE_H

#include <stdbool.h>
#include <stdint.h>

/* Retorna true se um caractere ascii for um digito(0-9) */
static inline bool isdigit(char c)
{
	return c >= '0' && c <= '9';
}

/* Retorna true se um caractere ascii for alpha(a-z ou A-Z) */
static inline bool isalpha(char c)
{
	return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

/* Retorna true se um caractere ascii for alpha ou digito */
static inline bool isalnum(char c)
{
	return isalpha(c) || isdigit(c);
}

/* Retorna true se o caractere ascii for ' ', '\t', '\n', '\r', '\v' ou '\f' */
static inline bool isspace(char c)
{
	return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\v' ||
		   c == '\f';
}

/* Retorna true se o caractere ascii for de A-Z */
static inline bool isupper(char c)
{
	return c >= 'A' && c <= 'Z';
}

/* Retorna true se o caractere ascii for de a-z */
static inline bool islower(char c)
{
	return c >= 'a' && c <= 'z';
}

/* Converte um caractere ascii para maiusculo */
static inline char toupper(char c)
{
	if (islower(c))
		return c - ('a' - 'A');
	return c;
}

/* Converte um caractere ascii para minusculo */
static inline char tolower(char c)
{
	if (isupper(c))
		return c + ('a' - 'A');
	return c;
}

#endif /* CTYPE_H */
