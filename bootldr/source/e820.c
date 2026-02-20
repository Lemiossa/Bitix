/************************************
 * e820.c                           *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#include <stdint.h>
#include "stdio.h"
#include "real_mode.h"
#include "e820.h"

/* Pega a tabela E820 */
/* Retorna o número de entradas, 1 se erro */
int E820_get_table(E820Entry *out, int max)
{
	int count = 0;
	Regs r = {0};
	do {
		E820Entry entry;
		r.d.eax = 0xE820;
		r.d.ecx = sizeof(E820Entry);
		r.d.edx = 0x534D4150;
		r.d.es = MK_SEG(&entry);
		r.w.di = MK_OFF(&entry);

		int16(0x15, &r);

		if (r.d.eax != 0x534D4150 || r.d.eflags & FLAG_CF)
			return 1;

		printf("E820_table[%d]: %08X-%08X:%d\r\n",
				count, (uint32_t)entry.base, (uint32_t)entry.length, entry.type);

		out[count++] = entry;
	} while (r.d.ebx != 0 && count < max);

	return count;
}

