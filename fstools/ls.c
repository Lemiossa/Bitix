/**
 * ls.c
 * Created by Matheus Leme Da Silva
 */
 
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdarg.h>
#include <errno.h>
#include <time.h>
#include "fs.h"

uint16_t block_size=1024;
int entries_per_block=0;
int bits_per_block=0;

#define MIN(x, y) ((x)<(y)?(x):(y))

void fatalf(const char *func, const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	fprintf(stderr, "\033[1;41mFatal: %s: ", func);
	vfprintf(stderr, fmt, args);
	fprintf(stderr, ": %s\033[0m\n", strerror(errno));
	va_end(args);
	exit(1);
}

#define FATAL(...) fatalf(__func__, __VA_ARGS__)

void rblock(int fd, uint32_t block, void *buf)
{
	if(lseek(fd, (off_t)block*block_size, SEEK_SET)<0)
		FATAL("Failed to seek block %u", block);
	if(read(fd, buf, block_size) != block_size)
		FATAL("Failed to read block %u", block);
}

void read_dirent(int fd, struct superblock *sb, uint32_t idx, struct dirent *de)
{
	uint8_t *buf=malloc(block_size);
	if (!buf)
		FATAL("Failed to alloc memory for dirent block");
	uint32_t block=sb->dirent_start+idx/entries_per_block;
	uint32_t offset=idx%entries_per_block;
	rblock(fd, block, buf);
	memcpy(de, buf+offset*DIRENT_SIZE, DIRENT_SIZE);
	free(buf);
}

uint32_t find_dirent(int fd, struct superblock *sb, uint32_t parent, const char *name)
{
	struct dirent de;
	for(uint32_t i=0;i<(sb->data_start-sb->dirent_start)*entries_per_block;i++) {
		read_dirent(fd, sb, i, &de);
		if(de.name[0]&&de.parent==parent&&strncmp(de.name, name, MAX_NAME)==0)
			return i;
	}
	return 0xffffffff;
}

uint32_t resolve_path(int fd, struct superblock *sb, const char *path)
{
	if (!path||path[0]!='/')
		return 0xffffffff;
	char *copy=strdup(path);
	char *tok=strtok(copy+1, "/");
	uint32_t current=0;
	while (tok) {
		uint32_t next=find_dirent(fd, sb, current, tok);
		if (next==0xffffffff) {
			free(copy);
			return 0xffffffff;
		}
		current=next;
		tok=strtok(NULL, "/");
	}
	free(copy);
	return current;
}

void print_mode(uint16_t mode)
{
	char str[11]={'-'};
	str[0]=IS_DIR(mode)?'d':'-';
	str[1]=(mode&MODE_U_R)?'r':'-';
	str[2]=(mode&MODE_U_W)?'w':'-';
	str[3]=(mode&MODE_U_X)?'x':'-';
	str[4]=(mode&MODE_G_R)?'r':'-';
	str[5]=(mode&MODE_G_W)?'w':'-';
	str[6]=(mode&MODE_G_X)?'x':'-';
	str[7]=(mode&MODE_O_R)?'r':'-';
	str[8]=(mode&MODE_O_W)?'w':'-';
	str[9]=(mode&MODE_O_X)?'x':'-';
	str[10]='\0';
	printf("%s ", str);
}

void list_dir(int fd, struct superblock *sb, uint32_t dir_idx)
{
	struct dirent de;
	for (uint32_t i=0;i<(sb->data_start-sb->dirent_start)*entries_per_block;i++) {
		read_dirent(fd, sb, i, &de);

		if (de.name[0]=='\0'||de.parent!=dir_idx)
			continue;

		print_mode(de.mode);

		if (de.uid==0||de.gid==0)
			printf("root ");
		else
			printf("user ");

		printf("%5u ", de.size);

		time_t mod=de.mdatetime;
		struct tm *t=localtime(&mod);
		char timestr[32];
		strftime(timestr, sizeof(timestr), "%Y-%m-%d %H:%M", t);

		printf("%s ", timestr);
		printf("%s\n", de.name);
	}
}

/**
 * Main
 */
int main(int argc, char **argv)
{
	if (argc<3) {
		fprintf(stderr, "Usage: %s <image> <path>\n", argv[0]);
		return 1;
	}

	int fd=open(argv[1], O_RDONLY);
	if (fd<0)
		FATAL("Failed to open image");

	struct superblock sb;
	off_t offset=(off_t)SUPERBLOCK_SECTOR*SECTOR_SIZE;
	if (lseek(fd, offset, SEEK_SET)<0||
	    read(fd, &sb, sizeof(struct superblock))!=sizeof(struct superblock)) {
		FATAL("Failed to read superblock");
	}


	block_size=sb.block_size;
	bits_per_block=block_size*8;
	entries_per_block=block_size/DIRENT_SIZE;

	uint32_t dir_idx=resolve_path(fd, &sb, argv[2]);
	if (dir_idx==0xffffffff)
		FATAL("Path not found");

	struct dirent target;
	read_dirent(fd, &sb, dir_idx, &target);

	if (!IS_DIR(target.mode))
		FATAL("Target is not a directory");

	list_dir(fd, &sb, dir_idx);
	close(fd);
	return 0;
}
