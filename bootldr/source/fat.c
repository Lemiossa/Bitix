/************************************
 * fat.c                            *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#include <stddef.h>
#include <stdint.h>
#include "string.h"
#include "stdio.h"
#include "util.h"
#include "disk.h"
#include "fat.h"

static uint32_t sectors_per_fat = 0;
static uint32_t total_sectors = 0;
static uint32_t root_dir_sectors = 0;
static uint32_t data_sectors = 0;

static uint32_t root_lba = 0;
static uint32_t data_lba = 0;
static uint32_t fat_lba = 0;

static uint32_t total_clusters = 0;

static uint8_t fat_type = 0;
static uint8_t current_disk = 0;

static bootsector_t bootsector;

/* Converte nome em formato fat para filename */
void fat_name_to_filename(char *fatname, char *out)
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
void fat_filename_to_fatname(char *filename, char *out)
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
	out[12] = 0;
}

/* Retorna 1 se o cluster é o fim */
static int fat_is_eof(uint32_t cluster)
{
	if (fat_type == 12)
		return cluster >= 0x00000FF8;
	else if (fat_type == 16)
		return cluster >= 0x0000FFF8;
	else if (fat_type == 32)
		return cluster >= 0x0FFFFFF8;
	return 1;
}

/* Converte um cluster em LBA */
/* ATENÇÃO: Você DEVE chamar fat_configure() antes disso */
static uint32_t fat_cluster_to_lba(uint32_t cluster)
{
	return ((cluster - 2) * bootsector.bpb.sectors_per_cluster) + data_lba;
}

/* Lê um cluster */
/* ATENÇÃO: Você DEVE chamar fat_configure() antes disso */
static uint32_t fat_read_cluster(uint32_t cluster)
{
	uint8_t buf[SECTOR_SIZE * 2];
	uint32_t offset = 0;

	if (fat_type == 12)
		offset = cluster + (cluster / 2);
	else if (fat_type == 16)
		offset = cluster * 2;
	else if (fat_type == 32)
		offset = cluster * 4;

	uint32_t sector = fat_lba + (offset / SECTOR_SIZE);
	uint32_t entry_offset = offset % SECTOR_SIZE;

	disk_read_sector(current_disk, buf, sector);
	disk_read_sector(current_disk, &buf[SECTOR_SIZE], sector+1);

	uint32_t val = 0;
	if (fat_type == 12) {
		uint32_t entry_val = *(uint32_t*)&buf[entry_offset];
		if (cluster & 1)
			val = entry_val >> 4;
		else
			val = entry_val & 0x0FFF;
		val &= 0x0FFF;
	} else if (fat_type == 16) {
		uint32_t entry_val = *(uint32_t*)&buf[entry_offset];
		val = entry_val;
		val &= 0xFFFF;
	} else if (fat_type == 32) {
		uint32_t entry_val = *(uint32_t*)&buf[entry_offset];
		val = entry_val;
		val &= 0x0FFFFFFF;
	}

	return val;
}

/* "Inicializa o sistema" FAT */
/* Na real só faz os calculos das globais em um disco específico num setor específico */
/* Retorna um número diferente de 0 se houver erro */
int fat_configure(int disk, uint32_t lba)
{
	uint8_t buf[SECTOR_SIZE];
	if (disk_read_sector(disk, buf, lba) != 0) {
		printf("fat_configure(): Falha ao ler disco: %d\r\n", disk);
		return 1;
	}

	memcpy(&bootsector, buf, sizeof(bootsector));

	if (bootsector.bpb.bytes_per_sector != SECTOR_SIZE) {/* Raro, porém é bom verificar,
												 não vou dar suporte a setores com tamanho diferente de 512 bytes */
		printf("fat_configure(): o numero de bytes por setor e invalido\r\n");
		return 1;
	}

	if (bootsector.bpb.sectors_per_fat16 == 0) {
		fat_type = 32;
		sectors_per_fat = bootsector.ebpb._32.sectors_per_fat32;
	} else {
		sectors_per_fat = bootsector.bpb.sectors_per_fat16;
	}

	total_sectors = bootsector.bpb.total_sectors16 == 0 ? bootsector.bpb.total_sectors32 : bootsector.bpb.total_sectors16;
	root_dir_sectors = ((bootsector.bpb.root_dir_entries * 32) + (SECTOR_SIZE - 1)) / SECTOR_SIZE;
	data_lba = lba + (bootsector.bpb.reserved_sectors + (bootsector.bpb.num_fats * sectors_per_fat) + root_dir_sectors);
	fat_lba = lba + bootsector.bpb.reserved_sectors;
	data_sectors = total_sectors - (bootsector.bpb.reserved_sectors + (bootsector.bpb.num_fats * sectors_per_fat) + root_dir_sectors);
	total_clusters = data_sectors / bootsector.bpb.sectors_per_cluster;
	current_disk = disk;
	root_lba = data_lba - root_dir_sectors;

	if (total_clusters < 4085) {
		fat_type = 12;
	} else if (total_clusters < 65525){
		fat_type = 16;
	} else {
		fat_type = 32;
	}

	return 0;
}

/* Lê um diretorio em FAT */
/* Retorna um número diferente de 0 se houver erro */
/* Se o cluster for zero, lê no root dir */
/* ATENÇÃO: Você DEVE chamar fat_configure() antes disso */
int fat_read_dir(uint32_t cluster, uint32_t index, fat_entry_t *out)
{
	uint32_t current_cluster = cluster;
	uint32_t current_index = 0;
	uint8_t buf[SECTOR_SIZE];

	if (cluster == 0 && fat_type == 32)
		current_cluster = bootsector.ebpb._32.root_dir_cluster;

	while (!fat_is_eof(current_cluster)) {
		uint32_t sectors = bootsector.bpb.sectors_per_cluster;
		if (current_cluster == 0)
			sectors = root_dir_sectors;

		for (uint32_t i = 0; i < sectors; i++) {
			uint32_t lba = i;
			if (current_cluster == 0)
				lba += root_lba;
			else
				lba += fat_cluster_to_lba(current_cluster);

			if (disk_read_sector(current_disk, buf, lba) != 0)
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

/* Lê N bytes de um arquivo a partir de sua entrada FAT em um offset específico */
/* Retorna o número de bytes lidos */
/* ATENÇÃO: Você DEVE chamar fat_configure() antes disso */
size_t fat_read(void *dest, fat_entry_t *entry, size_t offset, size_t n)
{
	if (!dest || !entry || n == 0 || offset > entry->file_size) {
		return 0;
	}

	if (offset + n > entry->file_size)
		n = entry->file_size - offset;

	size_t total = 0;
	size_t current_offset = 0;
	size_t remaining = n;
	uint32_t current_cluster = entry->cluster_low;
	if (fat_type == 32)
		current_cluster |= ((entry->cluster_high & 0x0FFF) << 8);
	uint8_t *d = (uint8_t *)dest;

	while (!fat_is_eof(current_cluster)) {
		uint8_t buf[SECTOR_SIZE];
		uint32_t lba = fat_cluster_to_lba(current_cluster);

		for (size_t i = 0; i < bootsector.bpb.sectors_per_cluster; i++) {
			if (disk_read_sector(current_disk, buf, lba+i) != 0)
				return 0;

			for (size_t j = 0; j < SECTOR_SIZE; j++) {
				if (remaining == 0)
					goto end;

				if (current_offset < offset) {
					current_offset++;
					continue;
				}

				d[total] = buf[j];

				total++;
				current_offset++;
				remaining--;
			}
		}

		current_cluster = fat_read_cluster(current_cluster);
	}
end:
	return total;
}
