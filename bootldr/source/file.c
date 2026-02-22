/************************************
 * file.c                           *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#include <stddef.h>
#include <stdint.h>
#include "stdio.h"
#include "string.h"
#include "file.h"
#include "util.h"
#include "disk.h"
#include "fat.h"

static int current_disk = -1;

/* Abre um arquivo no sistema FAT */
/* Retorna um número diferente de zero se houver erro */
int open(file_t *f, const char *path)
{
	if (!path || !f)
		return 1;

	char str[MAX_PATH] = {0};
	size_t len = strlen(path);
	if (len == 0 || len > MAX_PATH)
		return 1;
	memcpy(str, path, len);

	char *p = (char *)&str[0];

	char letter = 'C';
	if (p[1] == ':') {
		letter = to_upper(p[0]);
		p += 2;
	} else {
		letter = disk_get_letter(disk_find_drive(boot_drive));
	}

	int disk = disk_find_letter(letter);
	if (disk == -1)
		return 1;

	char *parts[MAX_PATH_PARTS];
	int count = get_path_parts(p, parts, MAX_PATH_PARTS);
	if (count == 0)
		return 1;

	if (fat_configure(disk, 0) != 0)
		return 1;

	fat_entry_t entry;
	uint16_t current_cluster = 0;
	for (int i = 0; i < count; i++) {
		char fatname[12] = {0};
		fat_filename_to_fatname(parts[i], (char *)fatname);

		uint32_t index = 0;
		while (1) {
			if (fat_read_dir(current_cluster, index++, &entry) != 0)
				return 1;

			char name[12] = {0};
			memcpy(name, &entry.name, 11);

			if (strcmp(fatname, name) == 0)
				break;
		}
		index = 0;

		if (i < (count - 1) && !(entry.attr & FAT_ATTR_DIR))
			return 1;

		current_cluster = entry.cluster_low;
	}

	memcpy(&f->entry, &entry, sizeof(fat_entry_t));
	f->pos = 0;
	f->disk = disk;

	current_disk = disk;

	return 0;
}

/* Lê um arquivo */
/* Retorna o número de bytes lidos */
size_t read(file_t *f, size_t n, void *dest)
{
	if (!f)
		return 0;

	if (current_disk != f->disk) {
		current_disk = f->disk;
		fat_configure(current_disk, 0);
	}

	/*
	printf("fat_read(%08x, %08x, %lu, %08x);\r\n", (uint32_t)dest, (uint32_t)&f->entry, f->pos, n);
	printf("f->pos = %lu\r\n", f->pos);
	*/
	return fat_read(dest, &f->entry, f->pos, n);
}

/* Muda a posição de um arquivo */
/* Retorna a posição atual(depois da mudança) */
size_t seek(file_t *f, size_t pos)
{
	if (!f)
		return 0;

	if (pos > f->entry.file_size)
		pos = f->entry.file_size;

	f->pos = pos;

	return f->pos;
}

