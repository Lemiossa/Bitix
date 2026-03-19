/************************************
 * vfs.h                            *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#ifndef VFS_H
#define VFS_H
#include <stdint.h>
#include <stdbool.h>

#define MAX_NAME 256

typedef struct fs
{
	/* Arquivos e pastas */
	bool      (*fexists)(void *data, const char *path);

	/* Arquivos */
	void     *(*fopen)(void *data, const char *path, uint32_t *length);
	uint32_t  (*fread)(void *data, void *internal, uint32_t offset, uint32_t n, void *d);
	uint32_t  (*fwrite)(void *data, void *internal, uint32_t offset, uint32_t n, void *s);
	void      (*fclose)(void *data, void *internal);

	/* Diretórios */
	void     *(*dopen)(void *data, const char *path);
	bool      (*dread)(void *data, void *internal, uint32_t index, void *out); /* True = diretório, False = arquivo */
	void      (*dclose)(void *data, void *internal);

	void *data;
} fs_t;

typedef struct file
{
	uint32_t length;
	uint32_t pos;
	void *internal;
	fs_t *fs;
} file_t;

typedef struct dir
{
	uint32_t pos;
	void *internal;
	fs_t *fs;
} dir_t;

typedef struct dirent
{
	char name[MAX_NAME];
	uint32_t length;
} dirent_t;

void vfs_register_fs(char drive, fs_t fs);
bool fexists(const char *path);
file_t *fopen(const char *path);
uint32_t fread(file_t *f, uint32_t n, void *d);
uint32_t fwrite(file_t *f, uint32_t n, void *s);
uint32_t fseek(file_t *f, uint32_t pos);
void fclose(file_t *f);
dir_t *dopen(const char *path);
bool dread(dir_t *d, dirent_t *out);
void dclose(dir_t *d);

#endif /* VFS_H */
