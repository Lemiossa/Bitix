/************************************
 * file.h                           *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#ifndef FILE_H
#define FILE_H
#include <stddef.h>
#include "fat.h"

#define MAX_PATH 512
#define MAX_PATH_PARTS 64

typedef struct file {
	fat_entry_t entry;
	size_t pos;
	int disk;
} file_t;

int open(file_t *f, const char *path);
size_t read(file_t *f, size_t n, void *dest);
size_t seek(file_t *f, size_t pos);

#endif /* FILE_H */
