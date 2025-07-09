#ifndef FS_H
#define FS_H

#include "disk.h"
#include "types.h"

#define MAX_NAME 20
#define MAX_PATH 16

typedef struct {
	uchar magic[4];
	ushort total_blocks;
	ushort inode_start;
	ushort data_start;
	ushort inode_count;
	ushort used_inodes;
	ushort free_blocks;
	ushort root_inode;
	uchar reserved[BLOCK - (4 + 2 * 7)];
} bfx_super_t;

typedef struct {
	ushort size;
	ushort size_blocks;
	ushort start;
	ushort created;
	ushort parent;
	uchar name[MAX_NAME];
	uchar mode;
} bfx_inode_t;

#define BFX_SUPER_SEC 32
#define TMP_BUF_SIZE 512

#define INPB (uint) (BLOCK / sizeof (bfx_inode_t))

/* Errors */
#define EIO -1
#define ENOTFOUND -2
#define EPERMDEN -3
#define ETOOLARGE -4
#define ENOMEM -5
#define ENOTFILE -6
#define EINVPATH -7

#define MODE_DIR 0x80
#define MODE_READ 0x04
#define MODE_WRITE 0x02
#define MODE_EXEC 0x01
#define MODE_MASK 0x07

#define IS_DIR(m) ((m) & MODE_DIR)
#define IS_FILE(m) (!IS_DIR (m))
#define HAS_READ(m) ((m) & MODE_READ)
#define HAS_WRITE(m) ((m) & MODE_WRITE)
#define HAS_EXEC(m) ((m) & MODE_EXEC)

bfx_super_t sb;

extern uchar tmpbuf[TMP_BUF_SIZE];

int bfx_mount ();
int read_inode (ushort idx, bfx_inode_t *out);
int name_match (bfx_inode_t *inode, uchar *name);
int parse_path (char *path, char components[MAX_PATH][MAX_NAME]);
ushort find_inode_in_dir (ushort dir_idx, char *name);
int bfx_readfile (char *path, ushort seg, ushort off, uchar need, uchar forbid);

/**
 * FD SYSTEM
 */

#define BUF1 0x8000
#define BUF2 0x9000
#define BUF_SIZE 0xffff

typedef struct {
	uptr_t buffer;
	ushort size;
	uchar used;
} fd_t;

#define MAX_FILES_OPENED 32

extern fd_t fd_table[MAX_FILES_OPENED];

void init_fd ();
int open (char *path);
int close (int fd);
int read (int fd, uptr_t buf, int count);

#endif /* FS_H */
