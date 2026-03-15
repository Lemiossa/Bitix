/************************************
 * vfs.h                            *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#ifndef VFS_H
#define VFS_H
#include <stdint.h>
#include <stdbool.h>

#define MAX_NAME 256

typedef struct vfs_fs
{
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
} vfs_fs_t;

typedef struct vfs_file
{
	uint32_t length;
	uint32_t pos;
	void *internal;
	vfs_fs_t *fs;
} vfs_file_t;

typedef struct vfs_dir
{
	uint32_t pos;
	void *internal;
	vfs_fs_t *fs;
} vfs_dir_t;

typedef struct vfs_dirent
{
	char name[MAX_NAME];
	uint32_t length;
} vfs_dirent_t;

void vfs_register_fs(char drive, vfs_fs_t fs);
vfs_file_t *vfs_open(const char *path);
uint32_t vfs_read(vfs_file_t *f, uint32_t n, void *d);
uint32_t vfs_write(vfs_file_t *f, uint32_t n, void *s);
uint32_t vfs_seek(vfs_file_t *f, uint32_t pos);
void vfs_close(vfs_file_t *f);
vfs_dir_t *vfs_opendir(const char *path);
bool vfs_readdir(vfs_dir_t *d, vfs_dirent_t *out);
void vfs_closedir(vfs_dir_t *d);

#endif /* VFS_H */
