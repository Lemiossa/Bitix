/************************************
 * stdio.h                          *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#ifndef STDIO_H
#define STDIO_H
#include <stdarg.h>
#include <stddef.h>

int vsnprintf(char *s, int n, const char *fmt, va_list args);
int vsprintf(char *s, const char *fmt, va_list args);
int snprintf(char *s, int n, const char *fmt, ...);
int sprintf(char *s, const char *fmt, ...);
int sscanf(const char *s, const char *fmt, ...);

#endif /* STDIO_H */
