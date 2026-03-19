/************************************
 * vfs.c                            *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#include <ctype.h>
#include <vfs.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <heap.h>
#include <string.h>

fs_t filesystems[26];

/* Registra um novo FS */
void vfs_register_fs(char drive, fs_t fs)
{
    if (!isalpha(drive))
		return;

	drive = toupper(drive);
	filesystems[drive - 'A'] = fs;
}

/* Retorna true se um arquivo existe */
bool fexists(const char *path)
{
	if (!path)
		return false;

	char drive = 'C'; /* Padrão é C */
	if (path[1] == ':')
	{
		drive = path[0];
		path += 2;
	}

	if (path[0] == '/')
		path++;

	int idx = drive - 'A';
	fs_t *fs = &filesystems[idx];
	if (!fs->exists)
		return false;

	return fs->exists(fs->data, path);
}

/* Abre um arquivo no VFS */
file_t *fopen(const char *path)
{
	if (!path)
		return NULL;

	char drive = 'C'; /* Padrão é C */
	if (path[1] == ':')
	{
		drive = path[0];
		path += 2;
	}

	if (path[0] == '/')
		path++;

	drive = toupper(drive);
	int idx = drive - 'A';

	if (!isalpha(drive) || !filesystems[idx].open)
		return NULL;

	file_t *f = alloc(sizeof(file_t));
	if (!f)
		return NULL;

	f->fs = &filesystems[idx];
	f->internal = filesystems[idx].open(f->fs->data, path, &f->length);
	f->pos = 0;

	if (!f->internal)
	{
		free(f);
		return NULL;
	}

	return f;
}

/* Lê uma quantidade de N de um F em um D */
uint32_t fread(file_t *f, uint32_t n, void *d)
{
	if (!f || !f->fs || !f->internal || !f->fs->read || !f->fs->data)
		return 0;

	if (n + f->pos > f->length)
	{
		n = f->length - f->pos;
	}

	uint32_t readed_bytes = f->fs->read(f->fs->data, f->internal, f->pos, n, d);
	f->pos += readed_bytes;
	return readed_bytes;
}

/* Lê uma quantidade de N de um F de um S */
uint32_t fwrite(file_t *f, uint32_t n, void *s)
{
	if (!f || !f->fs || !f->internal || !f->fs->write || !f->fs->data)
		return 0;

	uint32_t writed_bytes = f->fs->write(f->fs->data, f->internal, f->pos, n, s);
	f->pos += writed_bytes;
	if (f->pos > f->length)
		f->length = f->pos;

	return writed_bytes;
}

/* Altera o ponteiro dentro de um arquivo */
/* Retorna o ponteiro na posição nova */
uint32_t fseek(file_t *f, uint32_t pos)
{
	if (!f || !f->fs)
		return 0;

	if (pos > f->length)
	{
		pos = f->length;
	}

	f->pos = pos;
	return f->pos;
}

/* Fecha um arquivo e libera seus recursos */
void fclose(file_t *f)
{
	if (!f || !f->fs || !f->internal || !f->fs->close || !f->fs->data)
		return;

	f->fs->close(f->fs->data, f->internal);
	free(f);
}

/* Abre um diretório */
dir_t *dopen(const char *path)
{
	if (!path)
		return false;

	char drive = 'C'; /* Padrão é C */
	if (path[1] == ':')
	{
		drive = path[0];
		path += 2;
	}

	drive = toupper(drive);
	int idx = drive - 'A';

	if (!isalpha(drive) || !filesystems[idx].opendir)
		return NULL;


	dir_t *d = alloc(sizeof(dir_t));
	if (!d)
		return NULL;

	d->fs = &filesystems[idx];
	d->internal = filesystems[idx].opendir(d->fs->data, path);
	d->pos = 0;

	if (!d->internal)
	{
		free(d);
		return NULL;
	}

	return d;
}

/* Lê um diretório */
bool dread(dir_t *d, dirent_t *out)
{
	if (!d || !d->internal || !d->fs || !d->fs->data || !d->fs->readdir)
		return false;

	dirent_t dirent = {0};
	if (!d->fs->readdir(d->fs->data, d->internal, d->pos, &dirent))
		return false;

	memcpy(out, &dirent, sizeof(dirent));

	d->pos++;
	return true;
}

/* Fecha um diretório */
void dclose(dir_t *d)
{
	if (!d || !d->internal || !d->fs || !d->fs->data || !d->fs->closedir)
		return;

	d->fs->closedir(d->fs->data, d->internal);
	free(d);
}

