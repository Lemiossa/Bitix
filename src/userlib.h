#ifndef USERLIB_H
#define USERLIB_H

#include "types.h"

#define BLOCK 512
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

/* Errors */
#define EIO -1
#define ENOTFOUND -2
#define EPERMDEN -3
#define ETOOLARGE -4
#define ENOMEM -5
#define ENOTFILE -6
#define EINVPATH -7

#define KEY_ESC 0x100 + 0x01
#define KEY_UP 0x100 + 0x48
#define KEY_DOWN 0x100 + 0x50
#define KEY_LEFT 0x100 + 0x4C
#define KEY_RIGHT 0x100 + 0x4D

extern ushort _seg;
extern uchar tmpbuf[BLOCK];
extern bfx_super_t sb;

#define FILESEG (_seg + 0x1000)

#define MAX_ARGS 8
#define ARGV_BASE 0x0002
#define ARGS_SEG FILESEG

int syscall (int id, int arg1, int arg2, int arg3, int arg4, int arg5);
void putc (uchar c);
void puts (uchar *s);
void setcursor (uint x, uint y);
void clear_screen (uchar attr);
void setvid (int mode);
void printf (uchar *format, ...);
char *numtostr (ulong val, uchar *out, ushort base, bool neg);
void hexdump (void *data, uint size);
int readblock (ushort lba, uchar *buf);
int writeblock (ushort lba, uchar *buf);
void copy_bytes (void *dst, void *src, int n);
ulong getms ();
int bfx_readfile (uchar *path, ushort seg, ushort off, uchar need, uchar forbid);
uint streq (uchar *a, uchar *b);
uint strneq (uchar *a, uchar *b, uint n);
uint strlen (uchar *s);
char *strchr (uchar *s, int c);
char *strstr (uchar *haystack, uchar *needle);
void strcpy (uchar *dst, uchar *src);
void strcat (uchar *dst, uchar *src);
char *strrchr (uchar *s, int c);
void *memcpy (void *dst, void *src, uint size);
void lmemcpy (void *buf, ushort seg, ushort off, uint size);
void lrmemcpy (ushort seg, ushort off, void *buf, uint size);
void memset (void *ptr, uchar val, uint size);
void printseg (ushort seg, ushort off, uint size);
void putpixel (uint x, uint y, uchar color);
int getcwd (uchar *buf, int max_len);
void putxy (uchar x, uchar y, uchar chr, uchar attr);
void get_cursor_pos (uchar *x, uchar *y);

/* I/O */
void outportb (ushort port, uchar value);
uchar inportb (ushort port);

void lwriteb (ushort seg, ushort off, uchar val);
uchar lreadb (ushort seg, ushort off);
void lwritew (ushort seg, ushort off, ushort val);
ushort lreadw (ushort seg, ushort off);

#endif /* USERLIB_H */
