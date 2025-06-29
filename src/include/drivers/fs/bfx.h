#ifndef BFX_H
#define BFX_H

#include "types.h"

typedef struct {
  uchar magic[4];
  u16 total_blocks;
  u16 inode_start_block;
  u16 data_start_block;
  u16 free_blocks;
  u16 inode_count;
  u16 root_inode;
} bfx_super_t;

typedef struct {
  u16 flags;
  u16 size_blocks;
  u16 start;
  u16 created_date;
  u16 modified_date;
  u8 reserved[6];
  uchar name[16];
} bfx_inode_t;

#define BFX_SUPER_SEC 32
#define TMP_BUF_SIZE 512

#define BLOCK 512
#define INPB (int)(BLOCK/sizeof(bfx_inode_t))

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

/* Errors */
#define ERR_IO            -1
#define ERR_NOTFOUND      -2
#define ERR_PERMDEN       -3
#define ERR_TOOLARGE      -4

extern bfx_super_t sb;

int bfx_mount();
void bfx_list();

#endif /* BFX_H */
