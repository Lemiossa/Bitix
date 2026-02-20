/************************************
 * main.c                           *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>
#include "string.h"
#include "real_mode.h"
#include "stdio.h"
#include "disk.h"
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

typedef struct {
	uint8_t jmp[3];
	uint8_t oem_name[8];
	uint16_t bytes_per_sector;
	uint8_t sectors_per_cluster;
	uint16_t reserved_sectors;
	uint8_t num_fats;
	uint16_t root_dir_entries;
	uint16_t total_sectors16;
	uint8_t media_descriptor;
	uint16_t sectors_per_fat;
	uint16_t sectors_per_track;
	uint16_t num_heads;
	uint32_t hidden_sectors;
	uint32_t total_sectors32;
} __attribute__((packed)) fat_bpb_t;

typedef struct {
	uint16_t hour:5;
	uint16_t minutes:6;
	uint16_t seconds:5; /* Multiplicar por 2 */
} __attribute__((packed)) fat_time_t;

typedef struct {
	uint16_t year:7;
	uint16_t month:4;
	uint16_t day:5;
} __attribute__((packed)) fat_date_t;

typedef struct {
	uint8_t name[11];
	uint8_t attr;
	uint16_t res;
	fat_time_t ctime;
	fat_date_t cdate;
	fat_date_t adate;
	uint16_t cluster_high;
	fat_time_t mtime;
	fat_date_t mdate;
	uint16_t cluster_low;
	uint32_t file_size; /* Em bytes */
} __attribute__((packed)) fat_entry_t;

#define FAT_ATTR_RDOLY 0x01
#define FAT_ATTR_HIDDN 0x02
#define FAT_ATTR_SYSTM 0x04
#define FAT_ATTR_VOLID 0x08
#define FAT_ATTR_DIR   0x10
#define FAT_ATTR_ARCHV 0x20

uint32_t total_sectors = 0;
uint32_t root_dir_sectors = 0;
uint32_t data_sectors = 0;

uint32_t root_lba = 0;
uint32_t data_lba = 0;
uint32_t fat_lba = 0;

uint32_t total_clusters = 0;
uint8_t fat_type = 0;
uint8_t current_drive = 0;
fat_bpb_t bpb;

/* "Inicializa o sistema" FAT */
/* Na real só faz os calculos das globais em um disco específico num setor específico */
/* Retorna um número diferente de 0 se houver erro */
/* Por enquanto, suporta FAT12 e FAT16 */
int fat_init(uint8_t drive, uint32_t lba)
{
	uint8_t buf[SECTOR_SIZE];
	if (disk_read_sector(drive, buf, lba) != 0)
		return 1;

	memcpy(&bpb ,buf, sizeof(fat_bpb_t));

	if (bpb.sectors_per_fat == 0)  /* Possivelmente FAT32 ou só erro
									   Atualmente, sem suporte a FAT32 */
		return 1;

	if (bpb.bytes_per_sector != SECTOR_SIZE) /* Raro, porém é bom verificar,
												 não vou dar suporte a setores com tamanho diferente de 512 bytes */
		return 1;

	total_sectors = bpb.total_sectors16 == 0 ? bpb.total_sectors32 : bpb.total_sectors16;
	root_dir_sectors = ((bpb.root_dir_entries * 32) + (SECTOR_SIZE - 1)) / SECTOR_SIZE;
	data_lba = lba + (bpb.reserved_sectors + (bpb.num_fats * bpb.sectors_per_fat) + root_dir_sectors);
	fat_lba = lba + bpb.reserved_sectors;
	data_sectors = total_sectors - (bpb.reserved_sectors + (bpb.num_fats * bpb.sectors_per_fat) + root_dir_sectors);
	total_clusters = data_sectors / bpb.sectors_per_cluster;
	current_drive = drive;
	root_lba = data_lba - root_dir_sectors;

	if (total_clusters < 4085) {
		fat_type = 12;
	} else {
		fat_type = 16;
	}

	return 0;
}

/* Lê um cluster */
/* ATENÇÃO: Você DEVE chamar fat_init() antes disso */
uint16_t fat_read_cluster(uint16_t cluster)
{
	uint8_t buf[SECTOR_SIZE * 2];
	uint32_t offset = 0;

	if (fat_type == 12)
		offset = cluster + (cluster / 2);
	else if (fat_type == 16)
		offset = cluster * 2;

	uint32_t sector = fat_lba + (offset / SECTOR_SIZE);
	uint32_t entry_offset = offset % SECTOR_SIZE;

	disk_read_sector(current_drive, buf, sector);
	disk_read_sector(current_drive, &buf[SECTOR_SIZE], sector+1);

	uint16_t val = 0;
	if (fat_type == 12) {
		uint16_t entry_val = *(uint16_t*)&buf[entry_offset];
		if (cluster & 1)
			val = entry_val >> 4;
		else
			val = entry_val & 0x0FFF;
		val &= 0x0FFF;
	} else if (fat_type == 16) {
		uint16_t entry_val = *(uint16_t*)&buf[entry_offset];
		val = entry_val;
		val &= 0xFFFF;
	}

	return val;
}

/* lista o diretorio raiz do sistema FAT */
/* ATENÇÃO: Você DEVE chamar fat_init() antes disso */
void fat_list_root(void)
{
	uint8_t buf[SECTOR_SIZE];
	for (uint32_t i = 0; i < root_dir_sectors; i++) {
		uint32_t lba = i + root_lba;
		if (disk_read_sector(current_drive, buf, lba) != 0)
			return;

		for (int j = 0; j < 16; j++) {
			fat_entry_t *entry = (fat_entry_t *)&buf[j*32];
			if (entry->name[0] == 0x00)
				goto end;

			if (entry->name[0] == 0xE5)
				continue;

			for (int x = 0; x < 11; x++) {
				putc(entry->name[x]);
			}

			printf("\r\n");
		}
	}
end:
	printf("<end>\r\n");
}

/* Func principal do bootloader */
int main(void)
{
	vga_clear(0x07);
	printf("Ola mundo!\r\n");

	if (fat_init(boot_drive, 0) != 0) { /* FLOPPY */
		printf("Falha ao inicializar sistema FAT\r\n");
		goto halt;
	}

	fat_list_root();

halt:
	printf("Sistema travado. Por favor, reinicie.\r\n");
	while (1);
}

