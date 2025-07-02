#ifndef X86_H
#define X86_H

#include <types.h>

typedef struct {
  ushort ax, bx, cx, dx;
  ushort si, di;
} regs16_t;

extern uchar current_attr;
extern uint screen_width;
extern uint screen_height;

void putc(uchar c);

void outportb(ushort port, uchar value);
uchar inportb(ushort port);

void updateregs(regs16_t *in);
void getregs(regs16_t *out);

#endif /* X86_H */
