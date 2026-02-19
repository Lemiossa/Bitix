/************************************
 * main.c                           *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>
#include "real_mode.h"
#include "stdio.h"
#include "util.h"
#include "vga.h"

typedef struct E820Entry {
	uint64_t base;
	uint64_t length;
	uint32_t type;
	uint32_t attr;
} __attribute__((packed)) E820Entry;

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

		out[count++] = entry;
	} while (r.d.ebx != 0 && count < max);

	return count;
}

#define SECTOR_SIZE 512

/* Pega parâmetros de um disco usando o BIOS */
/* Retorna um número diferente de zero se houver erro */
int disk_get_parameters(uint8_t drive, uint16_t *cyl, uint8_t *hds, uint8_t *spt)
{
	Regs r = {0};
	r.b.ah = 0x08;
	r.b.dl = drive;
	int16(0x13, &r);

	if (r.d.eflags & FLAG_CF)
		return 1;

	if (cyl)
		*cyl = r.b.ch | (((r.b.cl & 0xC0) >> 6) << 8);

	if (hds)
		*hds = r.b.dh + 1;

	if (spt)
		*spt = r.b.cl & 0x3F;

	return 0;
}

extern uint8_t boot_drive;

/* Le n setores de um disco usando o BIOS */
/* Retorna um número diferente de zero se houver erro */
/* ATENÇÃO: N não pode ser mais de 59. dest deve ser abaixo de 1MiB */
int disk_read(uint8_t drive, void *dest, uint32_t lba, uint8_t n)
{
	if (!dest || n == 0 || n > 59)
		return 1;

	uint16_t cyl;
	uint8_t hds, spt;

	if (disk_get_parameters(drive, &cyl, &hds, &spt) != 0)
		return 1;

	if (lba > (cyl * hds * spt))
		return 1;

	/* Calculo lba -> chs */
	/* https://wiki.osdev.org/Disk_access_using_the_BIOS_(INT_13h) */
	uint32_t tmp = lba / spt;
	uint16_t cylinder = tmp / hds;
	uint8_t head = tmp % hds;
	uint8_t sector = (lba % spt) + 1;

	printf("LBA: %u, CHS: %hu,%hhu,%hhu\r\n", lba, cylinder, head, sector);

	Regs r = {0};
	r.b.ah = 0x02;
	r.b.al = n;
	r.b.ch = cylinder & 0xFF;
	r.b.cl = sector | ((cylinder >> 2) & 0xC0);
	r.b.dh = head;
	r.d.es = MK_SEG(dest);
	r.w.bx = MK_OFF(dest);
	r.b.dl = drive;
	int16(0x13, &r);

	if (r.d.eflags & FLAG_CF)
		return 1;

	return 0;
}

/* Func principal do bootloader */
int main(void)
{
	vga_clear(0x07);
	printf("Ola mundo!\r\n");

	uint8_t buf[SECTOR_SIZE];
	if (disk_read(boot_drive, buf, 0, 1) != 0) {
		printf("Falha ao ler setor 0 do disco!\r\n");
		goto halt;
	} else {
		for (int i = 0; i < SECTOR_SIZE; i++) {
			printf("%02X ", buf[i]);
			if ((i + 1) % 24 == 0)
				printf("\r\n");
		}
		printf("\r\n");
	}

halt:
	printf("Sistema travado. Por favor, reinicie.\r\n");
	while (1);
}

