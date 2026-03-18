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
	bool      (*exists)(void *data, const char *path);

	/* Arquivos */
	void     *(*open)(void *data, const char *path, uint32_t *length);
	uint32_t  (*read)(void *data, void *internal, uint32_t offset, uint32_t n, void *d);
	uint32_t  (*write)(void *data, void *internal, uint32_t offset, uint32_t n, void *s);
	void      (*close)(void *data, void *internal);

	/* Diretórios */
	void     *(*opendir)(void *data, const char *path);
	bool      (*readdir)(void *data, void *internal, uint32_t index, void *out); /* True = diretório, False = arquivo */
	void      (*closedir)(void *data, void *internal);

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
file_t *open(const char *path);
uint32_t read(file_t *f, uint32_t n, void *d);
uint32_t write(file_t *f, uint32_t n, void *s);
uint32_t seek(file_t *f, uint32_t pos);
void close(file_t *f);
dir_t *opendir(const char *path);
bool readdir(dir_t *d, dirent_t *out);
void closedir(dir_t *d);

#endif /* VFS_H */
