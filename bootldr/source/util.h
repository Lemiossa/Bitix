/************************************
 * util.h                           *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#ifndef UTIL_H
#define UTIL_H
#include <stdint.h>

extern uint16_t cursor_x, cursor_y;
extern uint8_t current_attributes;

void putc(char c);
void puts(const char *s);
int printf(const char *fmt, ...);
char to_upper(char c);
char to_lower(char c);
void str_upper(char *s);
void str_lower(char *s);
int get_path_parts(char *path, char **parts, int max);
void wait_us(uint32_t n);
void wait_ms(uint32_t n);
void wait(uint32_t n);
int kbhit(void);
unsigned char kbgetchar(void);
unsigned char kbgetsc(void);


#endif /* UTIL_H */
