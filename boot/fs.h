#pragma once

#include "types.h"

extern uint16_t block_size;

#define SECTOR_SIZE 512
#define MAX_BOOT_SECTORS 65
#define SUPERBLOCK_SECTOR 66
#define MAX_NAME 38
#define FS_MAGIC {'B', 'F', 'X', ' '} 

#define MODE_U_R (1<<8)
#define MODE_U_W (1<<7)
#define MODE_U_X (1<<6)
#define MODE_G_R (1<<5)
#define MODE_G_W (1<<4)
#define MODE_G_X (1<<3)
#define MODE_O_R (1<<2)
#define MODE_O_W (1<<1)
#define MODE_O_X (1<<0)
#define MODE_DIR (1<<15)

#define MODE_USER (MODE_U_R|MODE_U_W|MODE_U_X)
#define MODE_GROUP (MODE_G_R|MODE_G_W|MODE_G_X)
#define MODE_OTHER (MODE_O_R|MODE_O_W|MODE_O_X)
#define MODE_MASK (MODE_USER|MODE_GROUP|MODE_OTHER)

#define IS_DIR(m) ((m)&MODE_DIR)
#define IS_FILE(m) (!IS_DIR(m))

struct PACKED superblock {
	char magic[4];
	uint16_t block_size;
	uint32_t total_blocks;
	uint32_t bitmap_start;
	uint32_t dirent_start;
	uint32_t data_start;
};

struct PACKED dirent {
	uint32_t size;
	uint32_t start;
	uint32_t cdatetime;
	uint32_t mdatetime;
	uint32_t parent;
	uint16_t uid;
	uint16_t gid;
	uint16_t mode;
	char name[MAX_NAME];
};

#define DIRENT_SIZE sizeof(struct dirent)

void init_fs();

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

#define MAX_FILES 8

struct file {
    uint32_t dirent_idx;
    uint32_t offset;
    uint32_t size;
    uint32_t start;
    uint8_t in_use;
};

int open(const char *path);
int read(int fd, void *buf, uint32_t count);
int lseek(int fd, int32_t offset, int whence);
void close(int fd);
