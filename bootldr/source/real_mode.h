/************************************
 * real_mode.h                      *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#ifndef REAL_MODE_H
#define REAL_MODE_H
#include <stdint.h>

#define MK_SEG(ptr) ((uint16_t)((uint32_t)(ptr) >> 4))
#define MK_OFF(ptr) ((uint16_t)((uint32_t)(ptr) & 0x0F))

typedef union Regs {
	struct {
		uint8_t al, ah, __al, __ah;
		uint8_t bl, bh, __bl, __bh;
		uint8_t cl, ch, __cl, __ch;
		uint8_t dl, dh, __dl, __dh;
	} b;

	struct {
		uint16_t ax, __ax;
		uint16_t bx, __bx;
		uint16_t cx, __cx;
		uint16_t dx, __dx;
		uint16_t bp, __bp;
		uint16_t si, __si;
		uint16_t di, __di;
	} w;

	struct {
		uint32_t eax;
		uint32_t ebx;
		uint32_t ecx;
		uint32_t edx;
		uint32_t ebp;
		uint32_t esi;
		uint32_t edi;
		uint32_t ds;
		uint32_t es;
		uint32_t eflags;
	} d;
} Regs;

#define FLAG_CF (1 << 0)
#define FLAG_PF (1 << 2)
#define FLAG_AF (1 << 4)
#define FLAG_ZF (1 << 6)
#define FLAG_SF (1 << 7)
#define FLAG_TF (1 << 8)
#define FLAG_IF (1 << 9)
#define FLAG_DF (1 << 10)
#define FLAG_OF (1 << 11)
#define FLAG_NT (1 << 14)

void int16(uint8_t intnum, Regs *r);

#endif /* REAL_MODE_H */
