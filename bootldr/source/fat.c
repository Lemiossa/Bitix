/************************************
 * fat.c                            *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#include <stdint.h>
#include "string.h"
#include "stdio.h"
#include "util.h"
#include "disk.h"
#include "fat.h"

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

/* Converte nome em formato fat para filename */
static void fat_name_to_filename(char *fatname, char *out)
{
	for (int i = 0; i < 8; i++) {
		char c = fatname[i];
		if (c == ' ' || !c)
			break;
		*out++ = to_lower(c);
	}

	if (fatname[8] == ' ')
		return;

	*out++ = '.';

	for (int i = 0; i < 3; i++) {
		char c = fatname[i + 8];
		if (c == ' ' || !c)
			break;
		*out++ = to_lower(c);
	}
	*out = 0;
}

/* Converte um nome comum de arquivo em nome fat */
static void fat_filename_to_fatname(char *filename, char *out)
{
	for (int i = 0; i < 11; i++)
		out[i] = ' ';

	for (int i = 0; i < 8; i++) {
		char c = to_upper(*filename++);
		if (!c)
			return;

		if (c == '.')
			break;

		out[i] = c;
	}

	for (int i = 0; i < 3; i++) {
		char c = to_upper(*filename++);
		if (!c)
			return;

		out[i + 8] = c;
	}
}

/* Retorna 1 se o cluster é o fim */
static int fat_is_eof(uint16_t cluster)
{
	if (fat_type == 12)
		return cluster >= 0x0FF8;
	else if (fat_type == 16)
		return cluster >= 0xFFF8;
	return 1;
}

/* Converte um cluster em LBA */
/* ATENÇÃO: Você DEVE chamar fat_init() antes disso */
static uint16_t fat_cluster_to_lba(uint16_t cluster)
{
	return ((cluster - 2) * bpb.sectors_per_cluster) + fat_lba;
}

/* Lê um cluster */
/* ATENÇÃO: Você DEVE chamar fat_init() antes disso */
static uint16_t fat_read_cluster(uint16_t cluster)
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

/* Lê um diretorio em FAT */
/* Retorna um número diferente de 0 se houver erro */
/* ATENÇÃO: Você DEVE chamar fat_init() antes disso */
int fat_read_dir(uint16_t cluster, uint32_t index, fat_entry_t *out)
{
	uint16_t current_cluster = cluster;
	uint32_t current_index = 0;
	uint8_t buf[SECTOR_SIZE];

	while (!fat_is_eof(current_cluster)) {
		uint32_t sectors = bpb.sectors_per_cluster;
		if (current_cluster == 0)
			sectors = root_dir_sectors;

		for (uint32_t i = 0; i < sectors; i++) {
			uint32_t lba = i;
			if (current_cluster == 0)
				lba += root_lba;
			else
				lba += fat_cluster_to_lba(current_cluster);

			if (disk_read_sector(current_drive, buf, lba) != 0)
				return 1;

			fat_entry_t *entries = (fat_entry_t *)buf;
			for (uint32_t j = 0; j < SECTOR_SIZE / sizeof(fat_entry_t); j++) {
				if (entries[j].name[0] == 0x00)
					return 1;

				if (entries[j].name[0] == 0xE5)
					continue;

				if (current_index == index) {
					if (out)
						memcpy(out, &entries[j], sizeof(fat_entry_t));
					return 0;
				}

				current_index++;
			}
		}

		if (current_cluster == 0)
			return 1;

		current_cluster = fat_read_cluster(current_cluster);
	}
	return 1;
}

/* Lista a root dir */
void fat_list_root(void)
{
	uint32_t index = 0;

	while (1) {
		char filename[13] = {0};
		fat_entry_t entry;
		if (fat_read_dir(0, index++, &entry) != 0)
			break;

		if (entry.attr & FAT_ATTR_VOLID)
			continue;

		fat_name_to_filename((char *)entry.name, filename);
		printf("%s %ub \r\n", filename, entry.file_size);
	}
}

