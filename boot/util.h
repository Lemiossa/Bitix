#pragma once

#include "types.h"

void putc(char c);
void puts(const char* s);
void printf(const char* fmt, ...);

void Sprintf(const char* fmt, ...);

char *strcpy(char *dest, const char *src);
void *memset(void *dest, int val, size_t len);
int memcmp(const void *a, const void *b, size_t n);
void *memcpy(void *dest, const void *src, size_t n);
char *strncpy(char *dest, const char *src, size_t n);
int strncmp(const char *s1, const char *s2, size_t n);
char *strchr(const char *s, int c);
char *strtok(char *str, const char *delim);
size_t strlen(const char *s);
