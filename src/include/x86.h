#ifndef X86_H
#define X86_H

#include <types.h>

typedef union {
  struct {
    ushort ax, bx, cx, dx;
    short si, di;
  } x;
  struct {
    uchar al, ah, bl, bh;
    uchar cl, ch, dl, dh;
  } h;
} regs16_t;

typedef struct {
  uchar drive;
  uchar spt;
  uchar heads;
  uchar label[4];
} disk_t;

extern uchar drive;
extern disk_t disk;
extern uchar current_attr;
extern uint screen_width;
extern uint screen_height;
extern uchar video_mode;
extern ulong ticks;

void outportb(ushort port, uchar value);
uchar inportb(ushort port);

void lwrite(ushort seg, ushort off, uchar val);
uchar lread(ushort seg, ushort off);

void setregs(regs16_t *in);
void getregs(regs16_t *out);

int io_init_disk(uchar drive);
void reset_disk();
int io_readblock_chs(uchar head, uchar track, uchar sector, void *buf);

void init_pit(uint freq);

#endif /* X86_H */
