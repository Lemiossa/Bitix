/**
* mkfs.bfx.c
* Created by Matheus Leme Da Silva
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <math.h>

#define BLOCK 512
#define BOOT_BLOCKS 32
#define SUPERBLOCK_SECTOR BOOT_BLOCKS
#define MAX_NAME 22
#define FS_MAGIC "BFX"

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned short date16_t;

date16_t date; /* compacted date */

typedef struct {
  char magic[4];
  u16 total_blocks;
  u16 inode_start;
  u16 data_start;
  u16 inode_count;
  u16 used_inodes;
  u16 free_blocks;
  u16 root_inode;
  u8 reserved[BLOCK-(4+2*7)];
} superblock_t;

typedef struct {
  u8 mode;
  u16 size;
  u16 start;
  u16 created;
  u16 parent;
  char name[MAX_NAME];
} inode_t;

#define INODES_PER_BLOCK (BLOCK/sizeof(inode_t))

#define MODE_DIR 0x80
#define MODE_READ 0x04
#define MODE_WRITE 0x02
#define MODE_EXEC 0x01
#define MODE_MASK 0x07

#define IS_DIR(m) ((m)&MODE_DIR)
#define IS_FILE(m) (!IS_DIR(m))
#define HAS_READ(m) ((m)&MODE_READ)
#define HAS_WRITE(m) ((m)&MODE_WRITE)
#define HAS_EXEC(m) ((m)&MODE_EXEC)

/**
* Read a block from/to disk image
*/
static void rblock(int fd, u16 block, void *buf)
{
  lseek(fd, (off_t)block*BLOCK, SEEK_SET);
  read(fd, buf, BLOCK);
}

/**
* Write a block from/to disk image
*/
static void wblock(int fd, u16 block, void *buf)
{
  lseek(fd, (off_t)block*BLOCK, SEEK_SET);
  write(fd, buf, BLOCK);
}

/**
* Encode compacted date
*/
static date16_t encode_date(int year, int month, int day)
{
  year-=2000;
  if(year<0)year=0;
  if(year>127)year=127;
  if(month<1)month=1;
  if(month>12)month=12;
  if(day<1)day=1;
  if(day>31)day=31;

  return (year<<9)|(month<<5)|day;
}

/**
* Update the global compacted date
*/
void update_date16_now()
{
  time_t t=time(NULL);
  struct tm *tm=localtime(&t);
  date=encode_date(tm->tm_year+1900,tm->tm_mon+1,tm->tm_mday);
}

/**
* Get total blocks in image file
*/
static u16 get_total_blocks(int fd)
{
  int size=lseek(fd, 0, SEEK_END);
  return (u16)(size/BLOCK);
}

/**
* write a inode in the image
*/
static void write_inode(int fd, superblock_t *sb, u16 idx, inode_t *in)
{
  char buf[BLOCK];
  u16 block=sb->inode_start+idx/INODES_PER_BLOCK;
  u16 offset=idx%INODES_PER_BLOCK;
  rblock(fd, block, buf);
  ((inode_t*)buf)[offset]=*in;
  wblock(fd, block, buf);
}


/**
* Find inode index by name
*/
static u16 find_inode_by_name(int fd, superblock_t *sb, u16 dir_idx, const char *name)
{
  char buf[BLOCK];
  u16 block=sb->inode_start+dir_idx/INODES_PER_BLOCK;
  u16 off=dir_idx%INODES_PER_BLOCK;
  rblock(fd, block, buf);
  inode_t *dir=&((inode_t*)buf)[off];
  if(!IS_DIR(dir->mode)) return 0;

  for(u16 pos=0;pos<dir->size;pos+=sizeof(u16)) {
    u16 block=dir->start+pos/BLOCK;
    u16 offset=pos%BLOCK;
    rblock(fd, block, buf);
    u16 idx;
    memcpy(&idx, buf+offset,sizeof(u16));
    u16 cblk=sb->inode_start+idx/INODES_PER_BLOCK;
    u16 coff=idx%INODES_PER_BLOCK;
    rblock(fd, cblk, buf);
    inode_t *child=&((inode_t*)buf)[coff];
    if(strncmp(child->name, name, MAX_NAME)==0) return idx;
  }
  return 0;
}

/**
* add a dir entry in image
*/
static int add_dir_entry(int fd, superblock_t *sb, u16 dir_idx, u16 child_idx)
{
  char buf[BLOCK]={0};
  u16 blk=sb->inode_start+dir_idx/INODES_PER_BLOCK;
  u16 off=dir_idx%INODES_PER_BLOCK;

  rblock(fd, blk, buf);
  inode_t dir;
  memcpy(&dir, &((inode_t*)buf)[off], sizeof(inode_t));

  u16 pos=dir.size;
  u16 block=dir.start+pos/BLOCK;
  u16 offset=pos%BLOCK;

  rblock(fd, block, buf);
  memcpy(buf+offset, &child_idx, sizeof(u16));
  wblock(fd, block, buf);

  dir.size+=sizeof(u16);

  write_inode(fd, sb, dir_idx, &dir);
  return 0;
}

/**
* Create directory in the image
*/
static u16 create_dir(
  int fd,
  superblock_t *sb,
  const char *name,
  u8 perms,
  u16 parent,
  int *next_inode,
  u16 *next_data
)
{
  inode_t dir={
    .mode=MODE_DIR|(perms&MODE_MASK),
    .size=0,
    .start=(*next_data)++,
    .created=date,
    .parent=parent,
  };
  strncpy(dir.name, name, MAX_NAME-1);
  dir.name[MAX_NAME-1]=0;
  u16 idx=(*next_inode)++;
  write_inode(fd, sb, idx, &dir);
  sb->used_inodes++;
  sb->free_blocks--;
  if(parent!=0&&idx!=parent)
    add_dir_entry(fd, sb, parent, idx);
  return idx;
}

/**
* Parse the PATH
*/
static u16 resolve_path(
  int fd,
  superblock_t *sb,
  const char *path,
  int create,
  int *next_inode,
  u16 *next_data,
  u8 perms
)
{
  if(!path||!*path||path[0]!='/')
    return 0;
  char *copy=strdup(path);
  char *tok=strtok(copy+1,"/");
  u16 current=sb->root_inode;
  while(tok) {
    u16 next=find_inode_by_name(fd, sb, current, tok);
    if(next==0) {
      if(!create) {
        free(copy);
        return 0;
      }
      next=create_dir(fd, sb, tok, perms, current, next_inode, next_data);
    }
    current=next;
    tok=strtok(NULL, "/");
  }
  free(copy);
  return current;
}

int main(int argc, char **argv)
{
  if(argc<4) {
    fprintf(stderr, "Usage: %s <boot> <img> path:perms=hostfile ...\n", argv[0]);
    return 1;
  }

  const char *boot=argv[1], *img=argv[2];
  int imgfd=open(img, O_RDWR | O_CREAT, 0664);
  if(imgfd<0) {
    perror("open img");
    exit(1);
  }
  int bootfd=open(boot, O_RDONLY);
  if(bootfd<0) {
    perror("open boot");
    exit(1);
  }

  char buf[BLOCK]={0};
  for(int i=0;i<BOOT_BLOCKS;i++) {
    read(bootfd, buf, BLOCK);
    if(i==0)buf[510]=0x55,buf[511]=0xaa;
    wblock(imgfd, i, buf);
    memset(buf, 0,  BLOCK);
  }
  close(bootfd);

  u16 total_blocks=get_total_blocks(imgfd);
  u16 inode_blocks=(total_blocks*4)/100;
  u16 inode_count=inode_blocks*INODES_PER_BLOCK;
  u16 data_start=SUPERBLOCK_SECTOR+1+inode_blocks;

  superblock_t sb={
    .magic=FS_MAGIC,
    .total_blocks=total_blocks,
    .inode_start=SUPERBLOCK_SECTOR+1,
    .data_start=data_start,
    .inode_count=inode_count,
    .used_inodes=0,
    .free_blocks=total_blocks-data_start,
    .root_inode=1,
    .reserved={0},
  };

  int next_inode=1;
  u16 next_data=data_start;
  sb.root_inode=create_dir(imgfd, &sb, "/", 7, 0, &next_inode, &next_data);

  for(int i=3;i<argc;i++) {
    char *arg=strdup(argv[i]);
    char *eq=strchr(arg, '=');
    if(!eq) {
      free(arg);
      continue;
    }
    *eq=0;
    char *src=eq+1;
    char *colon=strchr(arg, ':');
    if(!colon) {
      free(arg);
      continue;
    }
    *colon=0;
    char *path=arg;
    u8 perms=strtol(colon+1, NULL, 8);

    const char *slash=strrchr(path, '/');
    const char *name=slash?slash+1:path;
    char *parent_path=slash?strndup(path, slash-path):strdup("/");
    u16 parent_inode=resolve_path(imgfd, &sb, parent_path, 1, &next_inode, &next_data, 6);
    if(!parent_inode) {
      fprintf(stderr, "Invalid path: %s\n", path);
      free(arg);
      free(parent_path);
      continue;
    }
    int sfd=open(src, O_RDONLY);
    if(sfd<0) {
      perror("src");
      free(arg);
      free(parent_path);
      continue;
    }
    int ffsize=lseek(sfd, 0, SEEK_END);
    if(ffsize>65536){
      fprintf(stderr, "File '%s' is too large\n", src);
      continue;
    }
    u16 fsize=ffsize;
    lseek(sfd, 0, SEEK_SET);
    u16 blocks_needed=(fsize+BLOCK-1)/BLOCK;
    if(blocks_needed>sb.free_blocks) {
      fprintf(stderr, "Not enough space for %s\n", src);
      close(sfd);
      free(arg);
      free(parent_path);
      continue;
    }
    u16 start=next_data;
    for(u16 j=0;j<blocks_needed;j++) {
      memset(buf, 0, BLOCK);
      read(sfd, buf, BLOCK);
      wblock(imgfd, start+j, buf);
    }
    close(sfd);

    inode_t file={
      .mode=0x00|(perms&MODE_MASK),
      .size=fsize,
      .start=start,
      .created=date,
      .parent=parent_inode,
    };
    strncpy(file.name, name, MAX_NAME-1);
    file.name[MAX_NAME-1]=0;

    u16 idx=next_inode++;
    write_inode(imgfd, &sb, idx, &file);
    add_dir_entry(imgfd, &sb, parent_inode, idx);
    next_data+=blocks_needed;
    sb.free_blocks-=blocks_needed;
    sb.used_inodes++;

    printf("Added %s\n", path);
    free(arg);
    free(parent_path);
  }
  wblock(imgfd, SUPERBLOCK_SECTOR, &sb);
  close(imgfd);
  printf("done. %hu inodes, ", inode_count);
  int total_bytes=total_blocks*BLOCK;
  if(total_bytes>=1024*1024) {
  	printf("%u MiB\n", total_bytes/1024/1024);
  } else if(total_bytes>=1024) {
  	printf("%u KiB\n", total_bytes/1024);
  } else {
  	printf("%u B\n", total_bytes);
  }
  return 0;
}
