/************************************
 * disk.c                           *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "util.h"
#include "disk.h"
#include "real_mode.h"

typedef struct disk {
	uint32_t cylinders, heads, sectors;
	uint32_t total_sectors;
	uint8_t drive;
	char letter;
} disk_t;

/*
 * 0x00 - 0x02: FDs
 * 0x80 - 0x87: HDs
 */
disk_t disks[MAX_DISKS];
int boot_disk = 0; /* Index do disco de boot */

/* Pega parâmetros de um disco usando o BIOS */
/* Retorna um número diferente de zero se houver erro */
/* ATENÇÃO: Você precisa chamar disk_dettect() primeiro! */
uint8_t disk_get_parameters(int disk, uint32_t *cylinders, uint32_t *heads, uint32_t *sectors)
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
	intx(0x13, &r);
	return r.b.ah;
}

uint8_t disk_buf[SECTOR_SIZE];

/* Le 1 setor de um disco usando o BIOS */
/* Retorna um número diferente de zero se houver erro */
uint8_t disk_read_sector(int disk, void *dest, uint32_t lba)
{
	if (!dest || disk > MAX_DISKS)
		return 1;

	uint32_t cylinders = disks[disk].cylinders;
	uint32_t heads = disks[disk].heads;
	uint32_t sectors = disks[disk].sectors;
	uint8_t drive = disks[disk].drive;

	if (lba > (cylinders * heads * sectors))
		return 1;

	/* Calculo lba -> chs */
	/* https://wiki.osdev.org/Disk_access_using_the_BIOS_(INT_13h) */
	uint32_t tmp = lba / sectors;
	uint32_t cylinder = tmp / heads;
	uint32_t head = tmp % heads;
	uint32_t sector = (lba % sectors) + 1;

	uint8_t ret = 0;
	for (int i = 0; i < 3; i++) {
		Regs r = {0};
		r.w.ax = 0x0201;
		r.b.ch = cylinder & 0xFF;
		r.b.cl = sector | ((cylinder >> 2) & 0xC0);
		r.b.dh = head;
		r.w.es = MK_SEG(disk_buf);
		r.w.bx = MK_OFF(disk_buf);
		r.b.dl = drive;
		intx(0x13, &r);

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
void disk_detect(void)
{
	int idx = 0;

	for (int i = 0x00; i < 0x02 && idx < MAX_DISKS; i++) { /* FDs */
		Regs r = {0};
		r.b.ah = 0x08;
		r.b.dl = i;
		intx(0x13, &r);

		if (r.w.flags & FLAG_CF) {
			disks[idx].letter = 0;
			continue;
		}

		char letter = 'A' + idx;

		disks[idx].cylinders = (r.b.ch | (((r.b.cl & 0xC0) >> 6) << 8)) + 1;
		disks[idx].heads = r.b.dh + 1;
		disks[idx].sectors = r.b.cl & 0x3F;
		disks[idx].total_sectors = disks[idx].cylinders * disks[idx].heads * disks[idx].sectors;
		disks[idx].drive = i;
		disks[idx].letter = letter;
		idx++;
	}

	for (int i = 0x80; i < (0x80 + (MAX_DISKS - 2)) && idx < MAX_DISKS; i++) { /* HDs */
		Regs r = {0};
		r.b.ah = 0x08;
		r.b.dl = i;
		intx(0x13, &r);

		if (r.w.flags & FLAG_CF)
			continue;

		char letter = 'A' + idx;

		disks[idx].cylinders = (r.b.ch | (((r.b.cl & 0xC0) >> 6) << 8)) + 1;
		disks[idx].heads = r.b.dh + 1;
		disks[idx].sectors = r.b.cl & 0x3F;
		disks[idx].total_sectors = disks[idx].cylinders * disks[idx].heads * disks[idx].sectors;
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

		printf("\tDisco %c: drive: 0x%02hhx, CHS: %hu,%hhu,%hhu, tamanho: ", disks[i].letter,
				disks[i].drive, disks[i].cylinders,
				disks[i].heads, disks[i].sectors);


		uint32_t bytes = disks[i].total_sectors * SECTOR_SIZE;

		/* vou usar Gb, Mb e Kb ao invez de Gib, Mib e Kib */
		if (bytes > 1000 * 1000 * 1000) {
			uint32_t gb_int = bytes / (1000 * 1000 * 1000);
			uint32_t gb_dec = (bytes % (1000 * 1000 * 1000)) / 100000; /* / 100000 para ter só 2 casas */
			printf("%u.%02u Gb", gb_int, gb_dec);
		} else if (bytes > 1000 * 1000) {
			uint32_t mb_int = bytes / (1000 * 1000);
			uint32_t mb_dec = (bytes % (1000 * 1000)) / 10000; /* / 10000 para ter só 2 casas */
			printf("%u.%02u Mb", mb_int, mb_dec);
		} else if (bytes > 1000) {
			uint32_t kb_int = bytes / 1000;
			uint32_t kb_dec = (bytes % 1000) / 1000; /* / 1000 para ter só 2 casas */
			printf("%u.%02u Kb", kb_int, kb_dec);
		} else {
			printf("%u bytes", bytes);
		}
		printf("\r\n");
	}
}
