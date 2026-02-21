/************************************
 * e820.c                           *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#include <stdint.h>
#include <string.h>
#include "real_mode.h"
#include "e820.h"
#include "util.h"

/* Pega a tabela E820 */
/* Retorna o número de entradas, 0 se erro */
int E820_get_table(e820_entry_t *out, int max)
{
	int count = 0;
	Regs r = {0};

	r.d.ebx = 0;
	do {
		printf(""); /* Não mecher, sei lá pq, mas só funciona se esse printf existir */
		r.d.eax = 0x0000E820;
		r.d.ecx = 24;
		r.d.edx = 0x534D4150;
		r.d.es = MK_SEG(&out[count]);
		r.w.di = MK_OFF(&out[count]);

		int15h(&r);

		if (r.d.eax != 0x534D4150 || r.d.eflags & FLAG_CF)
			return 0;

		printf("E820_table[%d]: 0x%08X-0x%08X:%d\r\n",
				count, (uint32_t)out[count].base, (uint32_t)out[count].length, out[count].type);

		count++;

		if (r.d.ebx == 0)
			break;
	} while (count < max);

	return count;
}

