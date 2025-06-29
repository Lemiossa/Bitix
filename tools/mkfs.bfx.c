/*
* mkfs.bfx.c
* Created by Matheus Leme Da Silva
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>

#define BLOCK                     512
#define MAX_BLOCK                 0xffff

typedef uint32_t   u32;
typedef uint16_t   u16;
typedef uint8_t    u8;

typedef struct {
  char magic[4];
  u16 total_blocks;
  u16 inode_start_block;
  u16 data_start_block;
  u16 free_blocks;
  u16 inode_count;
  u16 root_inode;
} __attribute__((packed)) superblock_t;

typedef struct {                                      /* ---------------------- */
  u16 flags;                                          /* Flags                  */
  u16 size_blocks;                                    /* Size in blocks         */
  u16 start;                                          /* Start block            */
  u16 created_date;                                   /* Created date           */
  u16 modified_date;                                  /* Modified date          */
  u8  reserved[6];                                    /* For complete 32 bytes  */
  char name[16];                                      /* Filename               */
} __attribute__((packed)) inode_t;                    /* ---------------------- */

#define INODES_PER_BLOCK  (int)(BLOCK / sizeof(inode_t))
#define SUPERBLOCK_SIZE sizeof(superblock_t)
#define TF(t, f)  ((t)|((f)<<3))
#define GT(tf)    ((tf)&7)
#define GF(tf)    (((tf)>>3)&7)

/* Types */
#define T_FREE    0
#define T_FILE    1
#define T_DIR     2
/* Perms */
#define P_READ    4
#define P_WRITE   2
#define P_EXEC    1

#define BOOT_SECTORS 32
#define SUPERBLOCK_SECTOR BOOT_SECTORS

void die(char *s) { printf("%s\n", s); exit(1); }
unsigned int porcentage(unsigned int percent, unsigned int total) { return (total*percent)/100; }

char *pflags(u16 flags) {
  static char mode[5];
  u8 type=GT(flags);
  u8 perms=GF(flags);
  mode[0]=(type==T_DIR)?'d':(type==T_FILE?'-':'?');
  mode[1]=(perms&P_READ)?'r':'-';
  mode[2]=(perms&P_WRITE)?'w':'-';
  mode[3]=(perms&P_EXEC)?'x':'-';
  mode[4]='\0';
  return mode;
}
u16 encode_date(u16 year, u8 month, u8 day)
{
  if(year<1980) year=1980;
  return ((year-1980)<<9)|(month<<5)|(day&0x1f);
}
void decode_date(u16 value, u16 *year, u8 *month, u8 *day)
{
  *year=1980+((value>>9)&0x7f);
  *month=(value>>5)&0x0f;
  *day=value&0x1f;
}
u16 get_current_packed_date()
{
  time_t now=time(NULL);
  struct tm *tm_info=localtime(&now);
  u16 year=tm_info->tm_year+1900;
  u8  month=tm_info->tm_mon+1;
  u8  day=tm_info->tm_mday;
  return encode_date(year, month, day);
}
char *date_to_string(u16 date)
{
  static char buffer[11];
  u16 year;
  u8 month, day;
  decode_date(date, &year, &month, &day);
  snprintf(buffer, sizeof(buffer), "%02u/%02u/%04u", day, month, year);
  return buffer;
}

void usage()
{
  printf("BFX: <command> <img> [args]\n");
  printf("  | format <img> <bootloader> <size in KB>\n");
  printf("  | list <img>\n");
  printf("  | add <img> <file> name [rwx]\n");
}

int format_fs(char *bootloader, char *image, int kb)
{
  printf("Formatting %s...\n", image);
  if(kb*2>MAX_BLOCK) return printf("Max 32MB\n"), 1;

  int img=open(image, O_CREAT|O_WRONLY|O_TRUNC, 0666);
  if(img<0) {
    fprintf(stderr, "Error in load %s.\n", image);
    return 1;
  }
  int boot=open(bootloader, O_RDONLY);
  if(boot<0) {
    fprintf(stderr, "Error in load %s.\n", bootloader);
    return 1;
  }

  lseek(img, kb*1024-1, SEEK_SET);
  write(img, "", 1);
  lseek(img, 0, SEEK_SET);

  char block[BLOCK]={0};
  int bytes_read;
  for(int j=0;j<BOOT_SECTORS;j++) {
    bytes_read=read(boot,block,BLOCK);
    if(j==0){ block[510]=0x55; block[511]=0xaa; }
    write(img,block,BLOCK);
    memset(block, 0, BLOCK);
  }
  close(boot);

  u16 total_blocks=kb*2;
  u16 inodes_blocks=porcentage(4, total_blocks);
  if(inodes_blocks<1) inodes_blocks=1;

  u16 inode_count=inodes_blocks*INODES_PER_BLOCK;
  u16 data_start=SUPERBLOCK_SECTOR+1+inodes_blocks;
  u16 free_blocks=total_blocks-data_start;

  superblock_t superblock={
    "BFX",
    total_blocks,
    SUPERBLOCK_SECTOR+1,
    data_start,
    free_blocks,
    inode_count,
    0
  };

  lseek(img,SUPERBLOCK_SECTOR*BLOCK,SEEK_SET);
  memset(block, 0, BLOCK);
  memcpy(block, &superblock, SUPERBLOCK_SIZE);
  write(img, block, BLOCK);

  //memset(block, 0, BLOCK);
  for(int j=0;j<inodes_blocks;j++)write(img, block, BLOCK);

  close(img);
  printf("data area start block %hu\n", superblock.data_start_block);
  printf("inode area start block %hu\n", superblock.inode_start_block);
  return printf("OK: %d blocks, %d inodes, %d bytes read\n",superblock.total_blocks, superblock.inode_count, bytes_read), 0;
}

int list_fs(char *image)
{
  int img=open(image, O_RDONLY);
  if(img<0)return 1;

  lseek(img, SUPERBLOCK_SECTOR*BLOCK, SEEK_SET);
  superblock_t superblock;
  read(img, &superblock, SUPERBLOCK_SIZE);

  /* Validate superblock */
  if(strcmp(superblock.magic, "BFX")) {
    printf("Filesystem magic is invalid %s", superblock.magic);
    close(img);
    return 1;
  }

  char bbuf[BLOCK];
  int files=0;

  printf("\n");

  int total_inode_blocks=(superblock.inode_count+15)/INODES_PER_BLOCK;

  for(int b=0;b<total_inode_blocks;b++) {
    lseek(img, (superblock.inode_start_block+b)*BLOCK, SEEK_SET);
    read(img, bbuf, BLOCK);

    inode_t *inodes=(inode_t*)bbuf;

    for(int j=0;j<INODES_PER_BLOCK;j++) {
      if(GT(inodes[j].flags)==T_FILE)
      {
        char *flags=pflags(inodes[j].flags);
        char *modified_date=date_to_string(inodes[j].modified_date);
        char *created_date=date_to_string(inodes[j].created_date);
        u32 size_bytes=inodes[j].size_blocks*BLOCK;
        printf(
          " %-4s %-10s %-10s %-15s %5u B -> block %5u\n",
          flags,
          modified_date,
          created_date,
          inodes[j].name,
          (u32)size_bytes,
          (u32)inodes[j].start
        );
        files++;
      }
    }
  }
  printf("\nTotal blocks: %d\nUsed blocks: %d\nFiles: %d\n\n", superblock.total_blocks, superblock.total_blocks-superblock.free_blocks, files);
  return close(img), 0;
}

int add_file(char *image, char *src, char *name, int f)
{
  int img=open(image, O_RDWR);
  int s=open(src, O_RDONLY);
  if(img<0||s<0) return 1;

  lseek(img, SUPERBLOCK_SECTOR*BLOCK, SEEK_SET);
  superblock_t superblock;
  read(img, &superblock, SUPERBLOCK_SIZE);

  /* Validate superblock */
  if(strcmp(superblock.magic, "BFX")) {
    printf("Filesystem magic is invalid %s", superblock.magic);
    close(img);
    return 1;
  }

  u32 size=lseek(s, 0, SEEK_END);
  lseek(s, 0, SEEK_SET);
  u16 need=(size+BLOCK-1)/BLOCK;

  /* Verify if have space in disk  */
  if(need>superblock.free_blocks) {
    printf("No space in disk (need %d blocks)\n", need);
    close(s);
    close(img);
    return 1;
  }

  if(strlen(name)>15) {
    printf("Filename is too long! (%zu bytes)\n", strlen(name));
    close(s);
    close(img);
    return 1;
  }

  char bbuf[BLOCK];
  int fb=-1,fo=-1;
  int total_inode_blocks=(superblock.inode_count+INODES_PER_BLOCK-1)/INODES_PER_BLOCK;

  for(int b=0;b<total_inode_blocks&&fb<0;b++) {
    lseek(img, (superblock.inode_start_block+b)*BLOCK, SEEK_SET);
    read(img, bbuf, BLOCK);

    inode_t *inodes=(inode_t*)bbuf;

    for(int j=0;j<INODES_PER_BLOCK;j++) {
      if(GT(inodes[j].flags)!=T_FREE&&strncmp(inodes[j].name, name, 16)==0) {
        printf("File '%s' already exist\n", name);
        close(s);
        close(img);
        return 1;
      }
      if(!GT(inodes[j].flags)) {
        fb=b;
        fo=j;
        break;
      }
    }
    if(fb>=0)break;
  }

  if(fb<0) {
    printf("No inodes\n");
    close(s);
    close(img);
    return 1;
  }

  u16 start=superblock.total_blocks-superblock.free_blocks;
  u16 current_date=get_current_packed_date();

  inode_t *inodes=(inode_t*)bbuf;
  inode_t *inode=&inodes[fo];

  inode->flags=TF(T_FILE, f);
  inode->size_blocks=need;
  inode->start=start;
  inode->created_date=current_date;
  inode->modified_date=current_date;
  memset(inode->name, 0, sizeof(inode->name));
  strncpy(inode->name, name, 16);
  inode->name[15]=0;

  lseek(img, (superblock.inode_start_block+fb)*BLOCK, SEEK_SET);
  write(img, bbuf, BLOCK);

  char buf[BLOCK];
  for(int j=0;j<need;j++) {
    memset(buf, 0, BLOCK);
    int r=read(s, buf, BLOCK);
    if(r<0) {
    	perror("read");
    	close(s);
    	close(img);
    	return 1;
    }
    if(r==0)break;
    if(r<BLOCK)memset(buf+r, 0, BLOCK-r);
    if(lseek(img, (start+j)*BLOCK, SEEK_SET)<0) {
    	perror("lseek");
    	close(s); close(img);
    	return 1;
    } 
    if(write(img, buf, BLOCK)!=BLOCK) {
    	perror("write");
    	close(s); close(img);
    	return 1;
    }
  }
  fsync(img);
	
  superblock.free_blocks-=need;
  lseek(img, SUPERBLOCK_SECTOR*BLOCK, SEEK_SET);
  write(img, &superblock, SUPERBLOCK_SIZE);
  close(s);
  close(img);
  return printf("Added file: %s (%d blocks)\n", name, need), 0;
}

int main(int c, char **v)
{
  if(c<3) return usage(), 1;
  if(!strcmp(v[1], "format")&&c==5)   return format_fs(v[2], v[3], atoi(v[4]));
  if(!strcmp(v[1], "list")&&c==3)     return list_fs(v[2]);
  if(!strcmp(v[1], "add")&&c>=4)      return add_file(v[2], v[3], c>=5?v[4]:v[3], c>=6?atoi(v[5]):7);
  return usage(), 1;
}
