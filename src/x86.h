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
extern ulong ticks;

void outportb (ushort port, uchar value);
uchar inportb (ushort port);

void lwriteb (ushort seg, ushort off, uchar val);
uchar lreadb (ushort seg, ushort off);
void lwritew (ushort seg, ushort off, ushort val);
ushort lreadw (ushort seg, ushort off);
int lcall (ushort seg, ushort off);

void setregs (regs16_t *in);
void getregs (regs16_t *out);

int io_init_disk (uchar drive);
void reset_disk ();
int io_readblock_chs (uchar head, uchar track, uchar sector, void *buf);
int io_writeblock_chs (u16 head, u16 track, u16 sector, void *buf);

void io_init_pit (uint freq);

int io_set_vesa_mode (ushort mode);
int io_set_vesa_bank (uint bank);

void sys_isr ();

#endif /* X86_H */
