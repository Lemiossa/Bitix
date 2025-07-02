#ifndef X86_H
#define X86_H

#include "types.h"

typedef struct {
  uchar drive;
  uchar spt;
  uchar heads;
  uchar label[4];
} disk_t;

extern uchar drive;
extern disk_t disk;

void lwrite8(u16 seg, u16 off, u8 val);
u8 lread8(u16 seg, u16 off);

void lwrite16(u16 seg, u16 off, u16 val);
u16 lread16(u16 seg, u16 off);

void ljump(u16 seg, u16 off);
void lcall(u16 seg, u16 off);

void hlt(void);
void cli(void);
void sti(void);

void outb(u16 port, u8 val);
u8 inb(u16 port);
void outw(u16 port, u16 val);
u16 inw(u16 port);
void outl(u16 port, u32 val);
u32 inl(u16 port);

void io_set_video_mode(u8 mode);

void io_set_8x8_font();

int io_init_disk(uchar drive);
void reset_disk();
int io_readblock_chs(uchar head, uchar track, uchar sector, void *buf);

void setax(u16 val);
void setbx(u16 val);
void setcx(u16 val);
void setdx(u16 val);

uint io_key_pressed();
uchar io_get_key();

void lgdt(void *gdt);
void enter_pm(ulong off);

#endif /* X86_H */
