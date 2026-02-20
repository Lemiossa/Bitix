/************************************
 * disk.c                           *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#include <stdint.h>
#include "stdio.h"
#include "string.h"
#include "disk.h"
#include "real_mode.h"

/* Pega parâmetros de um disco usando o BIOS */
/* Retorna um número diferente de zero se houver erro */
uint8_t disk_get_parameters(uint8_t drive, uint16_t *cyl, uint8_t *hds, uint8_t *spt)
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

	return r.b.ah;
}

/* Reseta um disco usando o BIOS */
/* Retorna um número diferente de zero se houver erro */
uint8_t disk_reset(uint8_t drive)
{
	Regs r = {0};
	r.b.ah = 0x00;
	r.b.dl = drive;
	int16(0x13, &r);
	return r.b.ah;
}

/* Le 1 setor de um disco usando o BIOS */
/* Retorna um número diferente de zero se houver erro */
/* ATENÇÃO: dest deve ser abaixo de 1MiB */
uint8_t disk_read_sector(uint8_t drive, void *dest, uint32_t lba)
{
	if (!dest)
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

	uint8_t ret = 0;
	for (int i = 0; i < 3; i++) {
		Regs r = {0};
		r.w.ax = 0x0201;
		r.b.ch = cylinder & 0xFF;
		r.b.cl = sector | ((cylinder >> 2) & 0xC0);
		r.b.dh = head;
		r.d.es = MK_SEG(dest);
		r.w.bx = MK_OFF(dest);
		r.b.dl = drive;
		int16(0x13, &r);
		ret = r.b.ah;

		if (ret == 0)
			break;

		if (disk_reset(drive) != 0)
			return 1;
	}


	return ret;
}

