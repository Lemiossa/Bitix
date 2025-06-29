#ifndef UTILS_H
#define UTILS_H

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
#define ERR_NOMEM         -5

#define KEY_ESC   0x100+0x01
#define KEY_UP    0x100+0x48
#define KEY_DOWN  0x100+0x50
#define KEY_LEFT  0x100+0x4C
#define KEY_RIGHT 0x100+0x4D

#define TICK_MS 10

extern ulong ticks;
extern bfx_super_t sb;

void ntos(u32 val, uchar *out, u16 base, bool neg);
void putcat(uchar c, uint x, uint y, uchar attr);
void puts(uchar *s);
void kputsf(uchar *format, ...);

int readblock(u16 lba, void *buf);

int cmpstr(uchar *s1, uchar *s2);
int lenofstr(uchar *str);

int bfx_mount();
void bfx_list();

uint getkey();

ulong get_ms();
ulong get_secs();
void sleep_ms(uint ms);

void putsxy(int x, int y, const uchar *s, uchar attr);

#endif /* UTILS_H */
