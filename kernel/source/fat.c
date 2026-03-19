/************************************
 * fat.c                            *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#include <stdio.h>
#include <vfs.h>
#include <ctype.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <heap.h>
#include <panic.h>
#include <ata.h>
#include <fat.h>
#include <terminal.h>
#include <debug.h>
#include <util.h>
#include <sched.h>

typedef struct fat_common_entry
{
	char name[256];
	uint8_t attr;
	uint16_t res;
	uint16_t ctime;
	uint16_t cdate;
	uint16_t adate;
	uint16_t cluster_high;
	uint16_t mtime;
	uint16_t mdate;
	uint16_t cluster_low;
	uint32_t file_size; /* Em bytes */
} fat_common_entry_t;

/* Converte nome em formato fat para filename */
void fat_name_to_filename(char *fatname, char *out)
{
	if (strlen(fatname) > 11)
	{
		strcpy(out, fatname);
		return;
	}

	for (int i = 0; i < 8; i++)
	{
		char c = fatname[i];
		if (c == ' ' || !c)
			break;
		*out++ = tolower(c);
	}

	if (fatname[8] == ' ')
		return;

	*out++ = '.';

	for (int i = 0; i < 3; i++)
	{
		char c = fatname[i + 8];
		if (c == ' ' || !c)
			break;
		*out++ = tolower(c);
	}
	*out = 0;
}

/* Converte um nome comum de arquivo em nome fat */
void fat_filename_to_fatname(char *filename, char *out)
{
	if (strlen(filename) > 12)
	{
		strcpy(out, filename);
		return;
	}

	for (int i = 0; i < 11; i++)
		out[i] = ' ';

	for (int i = 0; i < 8; i++)
	{
		char c = toupper(*filename++);
		if (!c)
			return;

		if (c == '.')
			break;

		out[i] = c;
	}

	for (int i = 0; i < 3; i++)
	{
		char c = toupper(*filename++);
		if (!c)
			return;

		out[i + 8] = c;
	}
	out[12] = 0;
}

/* Retorna o EOF val do FAT  */
static uint32_t fat_eof_val(fat_data_t *data)
{
	if (!data)
		panic("FAT: fat_eof_val(): Tentativa de chamar com data nula\r\n");

	if (data->fat_type == 12)
		return 0x00000FF8;
	else if (data->fat_type == 16)
		return 0x0000FFF8;
	else
		return 0x0FFFFFF8;
}

/* Retorna 1 se o cluster é o fim */
static int fat_is_eof(fat_data_t *data, uint32_t cluster)
{
	if (!data)
		panic("FAT: fat_is_eof(): Tentativa de chamar com data nula\r\n");

	return cluster >= fat_eof_val(data);
}

/* Retorna o cluster */
static uint32_t fat_cluster(fat_data_t *data, fat_common_entry_t e)
{
	if (!data)
		panic("FAT: fat_cluster(): Tentativa de chamar com data nula\r\n");

	if (data->fat_type == 32)
		return ((e.cluster_high & 0xFFF) << 16) | e.cluster_low;
	else if (data->fat_type == 16)
		return e.cluster_low;
	else
		return e.cluster_low & 0xFFF;
}

/* Converte um cluster em LBA */
/* ATENÇÃO: Você DEVE chamar fat_configure() antes disso */
static uint32_t fat_cluster_to_lba(fat_data_t *data, uint32_t cluster)
{
	if (!data)
		panic("FAT: fat_cluster_to_lba(): Tentativa de chamar com data nula\r\n");

	return ((cluster - 2) * data->bootsector.bpb.sectors_per_cluster) + data->data_lba;
}

/* Lê um valor na FAT */
/* ATENÇÃO: Você DEVE chamar fat_configure() antes disso */
static uint32_t fat_read_fat(fat_data_t *data, uint32_t i)
{
	if (!data)
		panic("FAT: fat_read_fat(): Tentativa de chamar com data nula\r\n");

	uint8_t buf[SECTOR_SIZE * 2];
	uint32_t offset = 0;

	if (data->fat_type == 12)
		offset = i + (i / 2);
	else if (data->fat_type == 16)
		offset = i * 2;
	else if (data->fat_type == 32)
		offset = i * 4;

	uint32_t sector = data->fat_lba + (offset / SECTOR_SIZE);
	uint32_t entry_offset = offset % SECTOR_SIZE;

	ata_read(data->current_disk, sector, 1, buf);
	ata_read(data->current_disk, sector + 1, 1, &buf[SECTOR_SIZE]);

	uint32_t val = 0;
	if (data->fat_type == 12)
	{
		uint32_t entry_val = *(uint32_t *)&buf[entry_offset];
		if (i & 1)
			val = entry_val >> 4;
		else
			val = entry_val & 0x0FFF;
		val &= 0x0FFF;
	}
	else if (data->fat_type == 16)
	{
		uint32_t entry_val = *(uint32_t *)&buf[entry_offset];
		val = entry_val;
		val &= 0xFFFF;
	}
	else if (data->fat_type == 32)
	{
		uint32_t entry_val = *(uint32_t *)&buf[entry_offset];
		val = entry_val;
		val &= 0x0FFFFFFF;
	}

	return val;
}

/* "Inicializa o sistema" FAT */
/* Na real só faz os calculos das globais em um disco específico num setor
 * específico */
/* Retorna um número diferente de 0 se houver erro */
static int fat_configure(fat_data_t *data, int disk, uint32_t lba)
{
	if (!data)
		panic("FAT: fat_configure(): Tentativa de chamar com data nula\r\n");

	uint8_t buf[SECTOR_SIZE];
	if (!ata_read(disk, lba, 1, buf))
	{
		debugf("FAT: fat_configure(): Falha ao ler disco: %d\r\n", disk);
		return 1;
	}

	memcpy(&data->bootsector, buf, sizeof(data->bootsector));

	if (data->bootsector.bpb.bytes_per_sector != SECTOR_SIZE)
	{ /* Raro, porém é bom verificar,
										  não vou dar suporte a setores com
		 tamanho diferente de 512 bytes */
		printf("fat_configure(): o numero de bytes por setor e invalido\r\n");
		return 1;
	}

	if (data->bootsector.bpb.sectors_per_fat16 == 0)
	{
		data->fat_type = 32;
		data->sectors_per_fat = data->bootsector.ebpb._32.sectors_per_fat32;
	}
	else
	{
		data->sectors_per_fat = data->bootsector.bpb.sectors_per_fat16;
	}

	data->total_sectors = data->bootsector.bpb.total_sectors16 == 0
						? data->bootsector.bpb.total_sectors32
						: data->bootsector.bpb.total_sectors16;
	data->root_dir_sectors =
		((data->bootsector.bpb.root_dir_entries * 32) + (SECTOR_SIZE - 1)) /
		SECTOR_SIZE;
	data->data_lba =
		lba + (data->bootsector.bpb.reserved_sectors +
			   (data->bootsector.bpb.num_fats * data->sectors_per_fat) + data->root_dir_sectors);
	data->fat_lba = lba + data->bootsector.bpb.reserved_sectors;
	data->data_sectors =
		data->total_sectors -
		(data->bootsector.bpb.reserved_sectors +
		 (data->bootsector.bpb.num_fats * data->sectors_per_fat) + data->root_dir_sectors);
	data->total_clusters = data->data_sectors / data->bootsector.bpb.sectors_per_cluster;
	data->current_disk = disk;
	data->root_lba = data->data_lba - data->root_dir_sectors;

	if (data->total_clusters < 4085)
	{
		data->fat_type = 12;
	}
	else if (data->total_clusters < 65525)
	{
		data->fat_type = 16;
	}
	else
	{
		data->fat_type = 32;
	}

	return 0;
}

/* Lê um diretorio em FAT */
/* Retorna 0 para sucesso, 1 para caso tenha acabado o diretorio e diferente caso haja erro */
/* Se o cluster for zero, lê no root dir */
/* ATENÇÃO: Você DEVE chamar fat_configure() antes disso */
static int fat_read_dir(fat_data_t *data,
		uint32_t cluster,
		uint32_t index,
		fat_common_entry_t *out)
{
	if (!data)
		panic("FAT: fat_read_dir(): Tentativa de chamar com data nula\r\n");

	uint32_t current_cluster = cluster;
	uint32_t current_index = 0;
	uint8_t buf[SECTOR_SIZE];

	if (cluster == 0 && data->fat_type == 32)
		current_cluster = data->bootsector.ebpb._32.root_dir_cluster;

	char lfn_name[256] = {0};
	int lfn = 0;
	while (!fat_is_eof(data, current_cluster))
	{
		uint32_t sectors = data->bootsector.bpb.sectors_per_cluster;
		if (current_cluster == 0)
			sectors = data->root_dir_sectors;

		for (uint32_t i = 0; i < sectors; i++)
		{
			uint32_t lba = i;
			if (current_cluster == 0)
				lba += data->root_lba;
			else
				lba += fat_cluster_to_lba(data, current_cluster);

			if (!ata_read(data->current_disk, lba, 1, buf))
				return 1;

			fat_entry_t *entries = (fat_entry_t *)buf;
			for (uint32_t j = 0; j < SECTOR_SIZE / sizeof(fat_entry_t); j++)
			{
				if (entries[j].name[0] == 0x00)
					return 1;

				if ((uint8_t)entries[j].name[0] == 0xE5)
				{
					if (lfn)
					{
						lfn = 0;
						memset(lfn_name, 0, sizeof(lfn_name));
					}

					continue;
				}

				if ((entries[j].attr & 0x0F) == 0x0F)
				{
					fat_lfn_entry_t *lfn_e = (fat_lfn_entry_t *)&entries[j];

					if (lfn_e->order & 0x40)
					{
						memset(lfn_name, 0, sizeof(lfn_name));
					}

					uint8_t order = lfn_e->order & 0x1F;
					int pos = (order - 1) * 13;

					for (int j = 0; j < 5; j++)
					{
						if (lfn_e->name1[j] == 0x0000 ||
								lfn_e->name1[j] == 0xFFFF)
							continue;

						lfn_name[pos++] = (char)lfn_e->name1[j];
					}

					for (int j = 0; j < 6; j++)
					{
						if (lfn_e->name2[j] == 0x0000 ||
								lfn_e->name2[j] == 0xFFFF)
							continue;

						lfn_name[pos++] = (char)lfn_e->name2[j];
					}


					for (int j = 0; j < 2; j++)
					{
						if (lfn_e->name3[j] == 0x0000 ||
								lfn_e->name3[j] == 0xFFFF)
							continue;

						lfn_name[pos++] = (char)lfn_e->name3[j];
					}

					lfn = 1;
					continue;
				}

				if (current_index == index)
				{
					if (out)
					{
					 	if (lfn)
						{
							strncpy((char *)&out->name, lfn_name, 256);
						}
						else
						{
							strncpy((char *)&out->name, (char *)&entries[j].name, 11);
						}

						memcpy(&out->attr, &entries[j].attr, sizeof(fat_entry_t) - 11);
					}

					return 0;
				}

				current_index++;
			}
		}

		if (current_cluster == 0)
			return 1;

		current_cluster = fat_read_fat(data, current_cluster);
	}
	return 1;
}

/* Lê N bytes de um arquivo a partir de sua entrada FAT em um offset específico */
/* Retorna o número de bytes lidos */
/* ATENÇÃO: Você DEVE chamar fat_configure() antes disso */
static size_t fat_read(fat_data_t *data,
		void *dest,
		fat_common_entry_t *entry,
		size_t offset,
		size_t n)
{
	if (!data)
		panic("FAT: fat_read(): Tentativa de chamar com data nula\r\n");

	if (!dest || !entry || n == 0 || offset > entry->file_size)
	{
		return 0;
	}

	if (offset + n > entry->file_size)
		n = entry->file_size - offset;

	size_t total = 0;
	size_t current_offset = 0;
	size_t remaining = n;
	uint32_t current_cluster = entry->cluster_low;
	if (data->fat_type == 32)
		current_cluster |= ((entry->cluster_high & 0x0FFF) << 8);
	uint8_t *d = (uint8_t *)dest;

	while (!fat_is_eof(data, current_cluster))
	{
		uint8_t buf[SECTOR_SIZE];
		uint32_t lba = fat_cluster_to_lba(data, current_cluster);

		for (size_t i = 0; i < data->bootsector.bpb.sectors_per_cluster; i++)
		{
			if (!ata_read(data->current_disk, lba + i, 1, buf))
				return 0;

			for (size_t j = 0; j < SECTOR_SIZE; j++)
			{
				if (remaining == 0)
					goto end;

				if (current_offset < offset)
				{
					current_offset++;
					continue;
				}

				d[total] = buf[j];

				total++;
				current_offset++;
				remaining--;
			}
		}

		current_cluster = fat_read_fat(data, current_cluster);
	}
end:
	return total;
}

/* Procura uma entrada a partir de uma path */
/* Retorna a entrada alocada */
static fat_common_entry_t *fat_find(fat_data_t *data, const char *path)
{
	if (!data || !path)
		return NULL;

	fat_common_entry_t entry;

	uint32_t len = strlen(path);

	char *tmp_path = alloc(len);
	if (!tmp_path)
		return NULL;
	strncpy(tmp_path, path, len);

	char *parts[64];
	int count = get_path_parts(tmp_path, parts, 64);
	if (count == 0)
	{
		free(tmp_path);
		return NULL;
	}

	uint32_t current_cluster = 0; /* Começa do root dir */
	for (int i = 0; i < count; i++)
	{
		char fatname[256] = {0};
		fat_filename_to_fatname(parts[i], (char *)fatname);

		uint32_t index = 0;
		while (1)
		{
			if (fat_read_dir(data, current_cluster, index++, &entry) != 0)
			{
				return NULL;
			}

			if (entry.attr & FAT_ATTR_VOLID)
				continue;

			char name[256] = {0};
			strncpy((char *)name, (char *)&entry.name, 256);
			if (strcmp(fatname, name) == 0)
				break;
		}

		if (i < (count - 1) && !(entry.attr & FAT_ATTR_DIR))
			return NULL;

		current_cluster = fat_cluster(data, entry);
	}

	fat_common_entry_t *e = alloc(sizeof(fat_common_entry_t));
	if (!e)
		return NULL;
	memset(e, 0, sizeof(*e));
	memcpy(e, &entry, sizeof(entry));

	return e;
}

/* Verifica se um arquivo existe em FAT */
static bool fat_fexists(void *data, const char *path)
{
	if (!data || !path)
		return false;

	fat_common_entry_t *entry = fat_find(data, path);
	if (!entry)
		return false;

	free(entry);

	return true;
}

/* Abre um arquivo em FAT */
static void *fat_fopen(void *data, const char *path, uint32_t *length)
{
	if (!data || !path)
		return NULL;

	fat_common_entry_t *entry = fat_find(data, path);
	if (!entry)
		return NULL;

	if (entry->attr & FAT_ATTR_DIR)
	{
		free(entry);
		return NULL;
	}

	if (length)
		*length = entry->file_size;

	return entry;
}

/* Lê um arquivo em FAT */
static uint32_t fat_fread(void *data, void *internal, uint32_t offset, uint32_t n, void *d)
{
	if (!data || !internal || !d)
		return 0;

	fat_common_entry_t *entry = internal;

	return fat_read(data, d, entry, offset, n);
}

/* Fecha um arquivo em FAT */
static void fat_fclose(void *data, void *internal)
{
	if (!internal)
		return;

	(void)data;

	free(internal);
}

/* Abre um diretório em FAT */
static void *fat_dopen(void *data, const char *path)
{
	if (!data || !path)
		return NULL;

	if (strcmp(path, "/") == 0 || path[0] == 0) /* Root dir */
	{
		fat_common_entry_t *entry = alloc(sizeof(fat_entry_t));
		if (!entry)
			return NULL;

		memset(entry, 0, sizeof(*entry));
		strcpy(entry->name, "/          ");
		entry->attr |= FAT_ATTR_DIR;
		return entry;
	}

	fat_common_entry_t *entry = fat_find(data, path);
	if (!entry)
		return NULL;

	if (entry->attr & FAT_ATTR_ARCHV)
	{
		free(entry);
		return NULL;
	}

	return entry;
}

/* Lê um diretório em FAT */
static bool fat_dread(void *data, void *internal, uint32_t index, void *out)
{
	if (!data || !internal || !out)
		return false;

	dirent_t dirent = {0};
	fat_common_entry_t e = {0};

	fat_common_entry_t *entry = internal;

	if (fat_read_dir(data, fat_cluster(data, *entry), index, &e) != 0)
		return false;

	if (e.attr & FAT_ATTR_ARCHV)
		dirent.length = e.file_size;

	fat_name_to_filename(e.name, (char *)dirent.name);
	memcpy(out, &dirent, sizeof(dirent));

	return true;
}

/* Fecha um diretório em FAT */
static void fat_dclose(void *data, void *internal)
{
	if (!internal)
		return;

	(void)data;

	free(internal);
}

/* Registra FAT em um disco */
/* Retorna TRUE se feito, FALSE se erro */
bool fat_registry(int disk, uint32_t start_lba, char letter)
{
	fat_data_t *data = alloc(sizeof(fat_data_t));
	if (!data)
		return false;

	fat_configure(data, disk, start_lba);

	fs_t fs = {
		.fexists = fat_fexists,
		.fopen = fat_fopen,
		.fread = fat_fread,
		.fwrite = NULL,
		.fclose = fat_fclose,
		.dopen = fat_dopen,
		.dread = fat_dread,
		.dclose = fat_dclose,
		.data = data
	};

	vfs_register_fs(letter, fs);

	return true;
}
