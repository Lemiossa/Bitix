/**
 * fs.c
 * Created by Matheus Leme Da Silva
 */ 

#include "types.h"
#include "fs.h"
#include "util.h"
#include "disk.h"

struct superblock sb;
uint16_t block_size=1024;
struct file files[MAX_FILES];

static uint8_t dirent_cache[8192];
static uint32_t cached_block=0xffffffff;

#define MAX_SECTORS_PER_READ 128

/**
 * Initialize FS
 * Read the superblock and copy to struct superblock sb;
 */
void init_fs()
{
	uint8_t sector_buf[SECTOR_SIZE];
	int ret=readsectors(SUPERBLOCK_SECTOR, sector_buf, 1);

	if(ret!=0) {
		printf("Failed to read superblock sector\r\n");
		return;
	}
	
	memcpy(&sb, sector_buf, sizeof(struct superblock));

	char expected_magic[4]=FS_MAGIC;
	
	if(memcmp(sb.magic, expected_magic, 4)!=0) {
		printf("Invalid superblock\r\n");
		return;
	}
	
	block_size=sb.block_size;
	for(int i=0;i<MAX_FILES;i++) {
		files[i].in_use=0;
	}
}

/**
 * Convert block to sector
 * Ex: 4K block -> 8 sectors
 */
static uint32_t block_to_sector(uint32_t block) 
{
    return block*(block_size/SECTOR_SIZE);
}

/**
 * Read a block 
 */
static int readblock(uint32_t block, void *buf) 
{
    uint32_t sector=block_to_sector(block);
    uint32_t sectors=block_size/SECTOR_SIZE;
    return readsectors(sector, buf, sectors);
}

/**
 * Read dirent
 */
static void read_dirent(uint32_t idx, struct dirent *de) 
{
    uint32_t entries_per_block=block_size/DIRENT_SIZE;
    uint32_t block=sb.dirent_start+idx/entries_per_block;
    uint32_t offset=(idx%entries_per_block)*DIRENT_SIZE;

    if (block!=cached_block) {
        if (readblock(block, dirent_cache)!=0) {
            return;
        }
        cached_block=block;
    }
    memcpy(de, dirent_cache+offset, DIRENT_SIZE);
}

/**
 * Find dirent in image
 */
static uint32_t find_dirent(const char *path) 
{
    if (!path||path[0]!='/')
        return 0xffffffff;

    char path_copy[MAX_NAME*4];
    strncpy(path_copy, path, sizeof(path_copy)-1);
    path_copy[sizeof(path_copy) - 1] = '\0';

    char *tok=strtok(path_copy+1, "/");
    uint32_t current_dirent_idx=0;
    struct dirent de;

    uint8_t block_buf[block_size];
    uint32_t entries_per_block=block_size/DIRENT_SIZE;

    while (tok) {
        uint32_t max_entries=(sb.data_start-sb.dirent_start)*entries_per_block;
        uint32_t next_dirent_idx=0xffffffff;

        uint32_t current_block=0xffffffff;

        for (uint32_t i=0; i<max_entries;i++) {
            uint32_t block_of_entry=sb.dirent_start+(i/entries_per_block);
            if (current_block!=block_of_entry) {
                if (readblock(block_of_entry, block_buf)!=0) {
                    return 0xffffffff; 
                }
                current_block=block_of_entry;
            }

            uint32_t offset_in_block=(i%entries_per_block)*DIRENT_SIZE;
            memcpy(&de, block_buf+offset_in_block, DIRENT_SIZE);

            if (de.name[0]=='\0')
                continue;

            if (de.parent==current_dirent_idx&&strncmp(de.name, tok, MAX_NAME)==0) {
                next_dirent_idx=i;
                break;
            }
        }

        if (next_dirent_idx==0xffffffff)
            return 0xffffffff;

        current_dirent_idx=next_dirent_idx;
        tok=strtok(NULL, "/");
    }
    return current_dirent_idx;
}

/**
 * Open a file
 */
int open(const char *path) 
{
    uint32_t dirent_idx=find_dirent(path);
    if (dirent_idx==0xffffffff) {
        printf("File not found: %s\r\n", path);
        return -1;
    }


    struct dirent de;
    read_dirent(dirent_idx, &de);
    if (IS_DIR(de.mode)) {
        printf("%s is a directory\r\n", path);
        return -1;
    }

    int fd=-1;
    for (int i=0;i<MAX_FILES;i++) {
        if (!files[i].in_use) {
            fd=i;
            break;
        }
    }
    
    if (fd==-1) {
        printf("No available file descriptors\r\n");
        return -1;
    }

    files[fd].dirent_idx=dirent_idx;
    files[fd].offset=0;
    files[fd].size=de.size;
    files[fd].start=de.start;
    files[fd].in_use=1;

    return fd;
}

/**
 * Read file from file descriptor
 */
int read(int fd, void *buf, uint32_t count) 
{
    if (fd<0||fd>=MAX_FILES||!files[fd].in_use) {
        printf("Invalid file descriptor\n");
        return -1;
    }

    struct file *f=&files[fd];
    if (f->offset>=f->size)
        return 0;

    uint32_t bytes_to_read=MIN(count, f->size-f->offset);
    uint32_t bytes_read=0;
    uint32_t sectors_per_block=block_size/SECTOR_SIZE;

    while (bytes_to_read>0) {
        uint32_t current_block_idx=f->offset/block_size;
        uint32_t offset_in_block=f->offset%block_size;
        uint32_t bytes_left_in_file=f->size-f->offset;
        uint32_t max_sectors_to_read=MAX_SECTORS_PER_READ;

        uint32_t sectors_in_remaining_blocks=(bytes_left_in_file+SECTOR_SIZE-1)/SECTOR_SIZE;
        uint32_t sectors_to_read=MIN(max_sectors_to_read, sectors_in_remaining_blocks);

        uint32_t bytes_possible=sectors_to_read*SECTOR_SIZE;
        if (offset_in_block+bytes_possible > block_size) {
            uint32_t blocks_possible=(bytes_possible+block_size-1)/block_size;
            sectors_to_read=blocks_possible*sectors_per_block;
            bytes_possible=sectors_to_read*SECTOR_SIZE;
        }

        if (bytes_possible>bytes_to_read) {
            sectors_to_read=(bytes_to_read+SECTOR_SIZE-1)/SECTOR_SIZE;
            bytes_possible=sectors_to_read*SECTOR_SIZE;
        }

        uint32_t start_sector=block_to_sector(f->start+current_block_idx)+(offset_in_block/SECTOR_SIZE);
        
        if (readsectors(start_sector, (uint8_t*)buf+bytes_read, sectors_to_read)!=0)
            return -1;

        uint32_t bytes_just_read=MIN(bytes_possible, bytes_to_read);
        f->offset+=bytes_just_read;
        bytes_read+=bytes_just_read;
        bytes_to_read-=bytes_just_read;
    }

    return bytes_read;
}

/**
 * Seek file
 * Set current file offset
 */
int lseek(int fd, int32_t offset, int whence) 
{
    if (fd<0||fd>=MAX_FILES||!files[fd].in_use) {
        printf("Invalid file descriptor\r\n");
        return -1;
    }

    struct file *f=&files[fd];
    uint32_t new_offset;

    switch (whence) {
        case 0: // SEEK_SET
            new_offset=offset;
            break;
        case 1: // SEEK_CUR
            new_offset=f->offset+offset;
            break;
        case 2: // SEEK_END
            new_offset=f->size+offset;
            break;
        default:
            printf("Invalid whence value\r\n");
            return -1;
    }

    if (new_offset>f->size) {
        printf("Seek beyond file size\r\n");
        return -1;
    }

    f->offset=new_offset;
    return new_offset;
}

/**
 * Close file
 */
void close(int fd) 
{
    if (fd<0||fd>=MAX_FILES||!files[fd].in_use) {
        printf("Invalid file descriptor\r\n");
        return;
    }
    files[fd].in_use=0;
}
