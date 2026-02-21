/************************************
 * string.h                         *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#ifndef STRING_H
#define STRING_H
#include <stddef.h>

void *memcpy(void *dest, const void *src, size_t n);
void memset(void *dest, int b, size_t n);
size_t strcmp(const char *s1, const char *s2);
size_t strncmp(const char *s1, const char *s2, size_t n);
size_t strlen(const char *s);
char *strchr(const char *s, char ch);
char *strtok(char *str, const char *delim);

#endif /* STRING_H */
