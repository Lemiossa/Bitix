/************************************
 * main.c                           *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#include <stdint.h>
#include <stdarg.h>
#include "stdio.h"
#include "util.h"
#include "vga.h"

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
		uint16_t ds;
		uint16_t es;
		uint16_t flags;
	} w;

	struct {
		uint32_t eax;
		uint32_t ebx;
		uint32_t ecx;
		uint32_t edx;
		uint32_t ebp;
		uint32_t esi;
		uint32_t edi;
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

typedef struct E820Entry {
	uint64_t base;
	uint64_t length;
	uint32_t type;
	uint32_t attr;
} __attribute__((packed)) E820Entry;

/* Pega a tabela E820 */
/* Retorna o número de entradas, -1 se erro*/
int E820_get_table(E820Entry *out, int max)
{
	int count = 0;
	Regs r = {0};
	do {
		E820Entry entry;
		r.d.eax = 0xE820;
		r.d.ecx = sizeof(E820Entry);
		r.d.edx = 0x534D4150;
		r.w.es = MK_SEG(&entry);
		r.w.di = MK_OFF(&entry);

		int16(0x15, &r);

		if (r.d.eax != 0x534D4150 || r.w.flags & FLAG_CF)
			return -1;
		out[count++] = entry;
	} while (r.d.ebx != 0 && count < max);

	return count;
}

int main(void)
{
	vga_clear(0x07);
	printf("Ola mundo!\r\n");

	printf("Sistema travado. Por favor, reinicie.\r\n");
	while (1);
}

