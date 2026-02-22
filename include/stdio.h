/************************************
 * stdio.h                          *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#ifndef STDIO_H
#define STDIO_H
#include <stdarg.h>

int vsprintf(char *o, const char *fmt, va_list args);
int sprintf(char *s, const char *fmt, ...);

#endif /* STDIO_H */
