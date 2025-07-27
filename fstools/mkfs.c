/**
 * mkfs.c
 * Created by Matheus Leme Da Silva
 */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include <stdarg.h>
#include "fs.h"

#define MIN(x, y) ((x)<(y)?(x):(y))

uint32_t cur_datetime;
uint16_t block_size=1024;
int bits_per_block=0;
int entries_per_block=0;

/**
 * To send a fatal error
 */
void fatalf(const char *func, const char *format, ...)
{
	va_list args;
	va_start(args, format);

	fprintf(stderr, "\033[41m");
	fprintf(stderr, "Fatal: %s: ", func);
	vfprintf(stderr, format, args);
	fprintf(stderr, ": %s\n", strerror(errno));
	fprintf(stderr, "\033[1;32m");

	va_end(args);
	exit(EXIT_FAILURE);
}

/**
 * To send an error 
 */
void errorf(const char *func, const char *format, ...)
{
	va_list args;
	va_start(args, format);

	fprintf(stderr, "\033[1;31m");
	fprintf(stderr, "Fatal: %s: ", func);
	vfprintf(stderr, format, args);
	fprintf(stderr, ": %s\n", strerror(errno));
	fprintf(stderr, "\033[1;32m");

	va_end(args);
}

#define FATAL(...) fatalf(__func__, __VA_ARGS__)
#define ERROR(...) errorf(__func__, __VA_ARGS__)

/**
 * Read a block of the image
 */
static void rblock(int fd, uint32_t block, void *buf)
{
	off_t offset=(off_t)block*block_size;
	if(lseek(fd, offset, SEEK_SET)<0) {
		close(fd);
		FATAL("Error seeking block %u in image %d", block, fd);
	}
	if(read(fd, buf, block_size)!=block_size) {
		close(fd);
		FATAL("Error reading block %u in image %d", block, fd);
	}
}

/**
 * Writes a block to the image
 */
static void wblock(int fd, uint32_t block, void *buf)
{
	off_t offset=(off_t)block*block_size;
	if(lseek(fd, offset, SEEK_SET)<0) {
		close(fd);
		FATAL("Error seeking block %u in image %d", block, fd);
	}
	if(write(fd, buf, block_size)!=block_size) {
		close(fd);
		FATAL("Error writing block %u in image %d", block, fd);
	}
}

/**
 * Updates the global date variable
 * 32-bit
 */
void update_datetime_now()
{
	cur_datetime=(uint32_t)time(NULL);
}

/**
 * Activates a bit within the bitmap
 */
void set_bitmap(int fd, struct superblock *sb, uint32_t block, int value)
{
	uint8_t *buf=malloc(block_size);
	if(!buf) {
		FATAL("Failed to alloc temporary buffer with %hu bytes\n", block_size);
	}
	memset(buf, 0, block_size);
	uint32_t bit_idx=block;
	uint32_t block_idx=sb->bitmap_start+bit_idx/bits_per_block;
	uint32_t offset=bit_idx%bits_per_block;
	rblock(fd, block_idx, buf);
	if(value)
		buf[offset/8]|=(1<<(offset%8));
	else 
		buf[offset/8]&=~(1<<(offset%8));
	wblock(fd, block_idx, buf);
	free(buf);
}

/**
 * Find the next free block
 */
int find_free_block(int fd, struct superblock *sb)
{
	uint8_t *buf=malloc(block_size);
	if(!buf) {
		FATAL("Failed to alloc temporary buffer with %hu bytes\n", block_size);
	}
	memset(buf, 0, block_size);

	for(uint32_t i=sb->data_start;i<sb->total_blocks;i++) {
		uint32_t block_idx=sb->bitmap_start+i/bits_per_block;
		uint32_t offset=i%bits_per_block;
		rblock(fd, block_idx, buf);
		if(!(buf[offset/8]&(1<<(offset%8)))) {
			set_bitmap(fd, sb, i, 1);
			return i;
		}
	}
	free(buf);
	return 0;
}

/**
 * Write a directive
 */
static void write_dirent(int fd, struct superblock *sb, uint32_t idx, struct dirent *de)
{
	uint8_t *buf=malloc(block_size);
	if(!buf) {
		FATAL("Failed to alloc temporary buffer with %hu bytes\n", block_size);
	}
	memset(buf, 0, block_size);
	uint32_t block=sb->dirent_start+idx/entries_per_block;
	uint32_t offset=idx%entries_per_block;
	rblock(fd, block, buf);
	memcpy(buf+offset*DIRENT_SIZE, de, DIRENT_SIZE);
	wblock(fd, block, buf);
	free(buf);
}

/**
 * Read a direction
 */
static void read_dirent(int fd, struct superblock *sb, uint32_t idx, struct dirent *de)
{
	uint8_t *buf=malloc(block_size);
	if(!buf) {
		FATAL("Failed to alloc temporary buffer with %hu bytes\n", block_size);
	}
	memset(buf, 0, block_size);
	uint32_t block=sb->dirent_start+idx/entries_per_block;
	uint32_t offset=idx%entries_per_block;
	rblock(fd, block, buf);
	memcpy(de, buf+offset*DIRENT_SIZE, DIRENT_SIZE);
	free(buf);
}

/**
 * Allocates a new dirent
 */
static uint32_t alloc_dirent(int fd, struct superblock *sb)
{
	struct dirent de;
	for(uint32_t i=0;i<(sb->data_start-sb->dirent_start)*entries_per_block;i++) {
		read_dirent(fd, sb, i, &de);
		if(i==0)
			continue;
		if(de.name[0]=='\0')
			return i;
	}
	return 0xffffffff;
}

/**
 * Search for a specific dirent in the image
 */
static uint32_t find_dirent(int fd, struct superblock *sb, uint32_t parent, const char *name)
{
	struct dirent de;
	for(uint32_t i=0;i<(sb->data_start-sb->dirent_start)*entries_per_block;i++) {
		read_dirent(fd, sb, i, &de);
		if(de.parent==parent&&strncmp(de.name, name, MAX_NAME)==0)
			return i;
	}
	return 0xffffffff;
}

/**
 * Create a directory in the image
 */
static uint32_t create_dir(int fd, struct superblock *sb, const char *name, uint16_t perms, uint32_t parent)
{
	uint32_t exists=find_dirent(fd, sb, parent, name);
	if(exists!=0xffffffff)
		return exists;
	uint32_t start=find_free_block(fd, sb);
	struct dirent de={
		.size=block_size,
		.start=start,
		.parent=parent,
		.cdatetime=cur_datetime,
		.mdatetime=cur_datetime,
		.uid=0,
		.gid=0,
		.mode=MODE_DIR|(perms&MODE_MASK)
	};
	strncpy(de.name, name, MAX_NAME);
	de.name[MAX_NAME-1]='\0';
	uint32_t idx=alloc_dirent(fd, sb);
	write_dirent(fd, sb, idx, &de);
	return idx;
}

/**
 * Deals with PATH
 * Create directories automatically
 */
static uint32_t resolve_path(int fd, struct superblock *sb, const char *path, int create)
{
	if(!path||path[0]!='/')
		return 0;
	char *copy=strdup(path);
	char *tok=strtok(copy+1, "/");
	uint32_t current=0;
	while(tok) {
		uint32_t next=find_dirent(fd, sb, current, tok);
		if(next==0xffffffff) {
			if(!create) {
				free(copy);
				return 0xffffffff;
			}
			next=create_dir(fd, sb, tok, 0755, current);
			if(!next) {
				free(copy);
				return 0xffffffff;
			}
		}
		current=next;
		tok=strtok(NULL, "/");
	}
	free(copy);
	return current;
}

/**
 * Main function
 */
int main(int argc, char **argv)
{
	if (argc<4) {
		fprintf (stderr, "Usage: %s <boot> <img> file:path:perms=hostfile ...\n", argv[0]);
		return 1;
	}
	for(int i=1;i<argc;i++) {
		if(strncmp(argv[i], "--block-size=", 13)==0) {
			const char *value_ptr=argv[i]+13;
			int val=atoi(value_ptr);
			if(val<512||val>8192||(val&(val-1))!=0) {
				ERROR("Error: Invalid block size (%d). Use powers of 2 between 512 and 8192.", val);
			}
			block_size=(uint16_t)val;
		}
	}
	bits_per_block=block_size*8;
	entries_per_block=block_size/DIRENT_SIZE;
	printf("\033[1;32m");
	
	/* Update the date */
	update_datetime_now();

	/* Open the images
	 * Bootloader and the image with FS
	 */
	const char *boot=argv[1], *img=argv[2];
	int imgfd=open (img, O_RDWR|O_CREAT, 0664);
	if (imgfd<0) {
		perror("open img");
		exit(1);
	}
	int bootfd=open(boot, O_RDONLY);
	if (bootfd<0) {
		perror("open boot");
		exit(1);
	}

	uint8_t sector_buf[SECTOR_SIZE]={0};
	for (int i=0;i<MAX_BOOT_SECTORS;i++) {
		ssize_t n=read(bootfd, sector_buf, SECTOR_SIZE);

		if(n==0) {
			memset(sector_buf, 0, SECTOR_SIZE);
		} else if(n<0) {
			close(imgfd);
			close(bootfd);
			FATAL("boot: Failed to read sector");
		} else if(n<SECTOR_SIZE) {
			memset(sector_buf+n, 0, SECTOR_SIZE-n);
		}
		
		if(i==0) {
			sector_buf[510]=0x55;
			sector_buf[511]=0xaa;
		}
		
		off_t offset=(off_t)i*SECTOR_SIZE;
		if(lseek(imgfd, offset, SEEK_SET)<0||
			write(imgfd, sector_buf, SECTOR_SIZE)!=SECTOR_SIZE) {
		   	close(imgfd);
		   	close(bootfd);
		   	FATAL("boot: Failed to write sector");
		}
	}
	close(bootfd);

	/* Write the bootloader to the image */
	uint8_t *buf=malloc(block_size);
	if(!buf) {
		FATAL("Failed to alloc temporary buffer with %hu bytes\n", block_size);
	}

	int blocks=lseek(imgfd, 0, SEEK_END)/block_size;
	uint32_t total_blocks=blocks;
	uint32_t bitmap_blocks=(total_blocks+bits_per_block-1)/bits_per_block;
	uint32_t dirent_blocks=(total_blocks*10)/100;

	struct superblock sb = {
		.magic=FS_MAGIC,
		.block_size=block_size,
		.total_blocks=total_blocks,
		.bitmap_start=SUPERBLOCK_SECTOR+1,
		.dirent_start=SUPERBLOCK_SECTOR+1+bitmap_blocks,
		.data_start=SUPERBLOCK_SECTOR+1+bitmap_blocks+dirent_blocks
	};

	for(uint32_t i=0;i<sb.data_start;i++)
		set_bitmap(imgfd, &sb, i, 1);
	
	struct dirent root={
		.size=block_size,
		.start=find_free_block(imgfd, &sb),
		.parent=0xffffffff,
		.cdatetime=cur_datetime,
		.mdatetime=cur_datetime,
		.uid=0,
		.gid=0,
		.mode=MODE_DIR|(0777&MODE_MASK),
	};
	memset(root.name, 0, MAX_NAME);

	write_dirent(imgfd, &sb, 0, &root);

	for (int i=3;i<argc;i++) {
		char *arg=strdup(argv[i]);
		if(!arg)
			continue;
		char *eq=strchr(arg, '=');
		if(!eq)
			continue;
		*eq=0;
		char *src=eq+1;
		char *colon=strchr(arg, ':');
		if(!colon)
			continue;
		*colon=0;
		char *path=arg;
		char *perm=colon+1;
		char *endptr=NULL;
		uint16_t mode=strtol(perm, &endptr, 8);
		if(*endptr!='\0'||mode>0777) {
			ERROR("Error in add file: Invalid permission: %s", perm);
			continue;
		}
		const char *slash=strrchr(path, '/');
		const char *name=slash?slash+1:path;
		char *parent=slash?strndup(path, slash-path):strdup("/");
		uint32_t pidx=resolve_path(imgfd, &sb, parent, 1);
		int ffd=open(src, O_RDONLY);
		int fsize=lseek(ffd, 0, SEEK_END);
		lseek(ffd, 0, SEEK_SET);
		uint32_t needed=(fsize+block_size-1)/block_size;
		uint32_t start=find_free_block(imgfd, &sb);
		for(uint32_t j=0;j<needed;j++) {
			memset(buf, 0, block_size);
			read(ffd, buf, block_size);
			wblock(imgfd, start+j, buf);
			set_bitmap(imgfd, &sb, start+j, 1);
		}
		close(ffd);
		struct dirent de = {
			.size=fsize,
			.start=start,
			.parent=pidx,
			.cdatetime=cur_datetime,
			.mdatetime=cur_datetime,
			.uid=0,
			.gid=0,
			.mode=(mode&MODE_MASK)
		};
		strncpy(de.name, name, MAX_NAME);
		de.name[MAX_NAME-1]='\0';
		uint32_t idx=alloc_dirent(imgfd, &sb);
		write_dirent(imgfd, &sb, idx, &de);
		printf("Added %s, %u bytes\n", path, fsize);
		free(parent);
		free(arg);
	}
	memset(buf, 0, block_size);
	memcpy(buf, &sb, sizeof(struct superblock));
	off_t offset=(off_t)SUPERBLOCK_SECTOR*SECTOR_SIZE;
	if (lseek(imgfd, offset, SEEK_SET)<0||
	    write(imgfd, &sb, sizeof(struct superblock))!=sizeof(struct superblock)) {
	    close(imgfd);
	    FATAL("Failed to write superblock at sector %d", SUPERBLOCK_SECTOR);
	}
	free(buf);
	fsync (imgfd);
	close (imgfd);

	/* Additional info */
	printf ("done. %u dirents, ", dirent_blocks*entries_per_block);
	unsigned int total_bytes=total_blocks*block_size;
	double kib=total_bytes/1024.0;
	double mib=kib/1024.0;

	if(total_bytes>=1024*1024) {
		printf("%.2f MiB\n", mib);
	} else if(total_bytes>=1024) {
		printf("%.2f KiB\n", kib);
	} else {
		printf("%u B\n", total_bytes);
	}
	printf("\033[0m");

	return 0;
}
