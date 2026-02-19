/************************************
 * stdio.h                          *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#ifndef STDIO_H
#define STDIO_H
#include <stdarg.h>

void putc(char c);
void puts(const char *s);
int vsprintf(char *out, const char *fmt, va_list args);
int printf(const char *fmt, ...);

#endif /* STDIO_H */
