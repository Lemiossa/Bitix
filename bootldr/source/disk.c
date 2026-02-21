/************************************
 * disk.c                           *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#include <stdint.h>
#include "string.h"
#include "stdio.h"
#include "util.h"
#include "disk.h"
#include "real_mode.h"

typedef struct disk {
	uint16_t cylinders;
	uint8_t heads, sectors;
	uint8_t drive;
	char letter;
} disk_t;

/*
 * 0x00 - 0x02: FDs
 * 0x80 - 0x8F: HDs
 */
disk_t disks[MAX_DISKS];
int boot_disk = 0; /* Index do disco de boot */

/* Pega parâmetros de um disco usando o BIOS */
/* Retorna um número diferente de zero se houver erro */
/* ATENÇÃO: Você precisa chamar disk_dettect() primeiro! */
uint8_t disk_get_parameters(int disk, uint16_t *cylinders, uint8_t *heads, uint8_t *sectors)
{
	if (disk > MAX_DISKS)
		return 1;

	if (cylinders)
		*cylinders = disks[disk].cylinders;

	if (heads)
		*heads = disks[disk].heads;

	if (sectors)
		*sectors = disks[disk].sectors;

	return 0;
}

/* Reseta um disco usando o BIOS */
/* Retorna um número diferente de zero se houver erro */
uint8_t disk_reset(int disk)
{
	if (disk > MAX_DISKS)
		return 1;
	Regs r = {0};
	r.b.ah = 0x00;
	r.b.dl = disks[disk].drive;
	int16(0x13, &r);
	return r.b.ah;
}

uint8_t disk_buf[SECTOR_SIZE];

/* Le 1 setor de um disco usando o BIOS */
/* Retorna um número diferente de zero se houver erro */
uint8_t disk_read_sector(int disk, void *dest, uint32_t lba)
{
	if (!dest || disk > MAX_DISKS)
		return 1;

	memset(disk_buf, 0, SECTOR_SIZE);

	uint16_t cylinders = disks[disk].cylinders;
	uint8_t heads = disks[disk].heads;
	uint8_t sectors = disks[disk].sectors;
	uint8_t drive = disks[disk].drive;

	if (lba > (cylinders * heads * sectors))
		return 1;

	/* Calculo lba -> chs */
	/* https://wiki.osdev.org/Disk_access_using_the_BIOS_(INT_13h) */
	uint32_t tmp = lba / sectors;
	uint16_t cylinder = tmp / heads;
	uint8_t head = tmp % heads;
	uint8_t sector = (lba % sectors) + 1;

	uint8_t ret = 0;
	for (int i = 0; i < 3; i++) {
		Regs r = {0};
		r.w.ax = 0x0201;
		r.b.ch = cylinder & 0xFF;
		r.b.cl = sector | ((cylinder >> 2) & 0xC0);
		r.b.dh = head;
		r.d.es = MK_SEG(disk_buf);
		r.w.bx = MK_OFF(disk_buf);
		r.b.dl = drive;
		int16(0x13, &r);
		ret = r.b.ah;

		if (ret == 0) {
			memcpy(dest, disk_buf, SECTOR_SIZE);
			break;
		}

		if (disk_reset(drive) != 0)
			return 1;
	}


	return ret;
}

/* Procura um disco de acordo com a letra */
/* Retorna -1 se houver erro */
int disk_find_letter(char letter)
{
	for (int i = 0; i < MAX_DISKS; i++) {
		if (disks[i].letter == letter)
			return i;
	}

	return -1;
}

/* Procura um disco de acordo com o drive */
/* Retorna -1 se houver erro */
int disk_find_drive(uint8_t drive)
{
	for (int i = 0; i < MAX_DISKS; i++) {
		if (disks[i].drive == drive)
			return i;
	}

	return -1;
}

/* Retorna a letra de um disco */
/* Retorna 0 se não existir */
char disk_get_letter(int disk)
{
	if (disk > MAX_DISKS)
		return 0;
	return disks[disk].letter;
}

/* Detecta todos os discos e atribui letra a eles */
void disk_dettect(void)
{
	int idx = 0;

	for (int i = 0x00; i < 0x02 && idx < MAX_DISKS; i++) { /* FDs */
		Regs r = {0};
		r.b.ah = 0x08;
		r.b.dl = i;
		int16(0x13, &r);

		if (r.d.eflags & FLAG_CF) {
			disks[idx].letter = 0;
			continue;
		}

		char letter = idx + 'A';

		disks[idx].cylinders = (r.b.ch | (((r.b.cl & 0xC0) >> 6) << 8)) + 1;
		disks[idx].heads = r.b.dh + 1;
		disks[idx].sectors = r.b.cl & 0x3F;
		disks[idx].drive = i;
		disks[idx].letter = letter;
		idx++;
	}

	for (int i = 0x80; i < (0x80 + (MAX_DISKS - 2)) && idx < MAX_DISKS; i++) { /* HDs */
		Regs r = {0};
		r.b.ah = 0x08;
		r.b.dl = i;
		int16(0x13, &r);

		if (r.d.eflags & FLAG_CF)
			continue;

		char letter = idx + 'C';

		disks[idx].cylinders = (r.b.ch | (((r.b.cl & 0xC0) >> 6) << 8)) + 1;
		disks[idx].heads = r.b.dh + 1;
		disks[idx].sectors = r.b.cl & 0x3F;
		disks[idx].drive = i;
		disks[idx].letter = letter;
		idx++;
	}

	printf("Discos: \r\n");
	for (int i = 0; i < MAX_DISKS; i++) {
		if (!disks[i].letter)
			continue;

		if (disks[i].drive == boot_drive)
			boot_disk = i;

		printf("\tDisco %c: \r\n", disks[i].letter);
		printf("\t\tdrive: 0x%02hhx\r\n", disks[i].drive);
		printf("\t\tcilindros: %hu\r\n", disks[i].cylinders);
		printf("\t\tcabecas: %hhu\r\n", disks[i].heads);
		printf("\t\tsetores: %hhu\r\n", disks[i].sectors);
	}
}
