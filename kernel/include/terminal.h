/************************************
 * terminal.h                       *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#ifndef TERMINAL_H
#define TERMINAL_H
#include <stdint.h>

#define TERMINAL_CURSOR_BLINK_MS 320

void terminal_init(void);
void terminal_set_cursor(int x, int y);
void terminal_clear(uint8_t fg, uint8_t bg);
void terminal_putchar(char c);
void terminal_putstring(const char *s);
int printf(const char *fmt, ...);

#endif /* TERMINAL_H */
