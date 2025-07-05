/**
* kernel.c
* Created by Matheus Leme Da Silva
*/

#include <types.h>
#include <x86.h>
#include <utils.h>

/**
* Remap PIC
*/
void pic_remap()
{
  outportb(0x20, 0x11);
  outportb(0xa0, 0x11);
  outportb(0x21, 0x20);
  outportb(0xa1, 0x28);
  outportb(0x21, 0x04);
  outportb(0xa1, 0x02);
  outportb(0x21, 0x01);
  outportb(0xa1, 0x01);
}

/**
* Set VGA VIDEO/TEXT mode
*/
void io_set_video_mode(uchar mode)
{
  regs16_t r;
  r.h.ah=0x00;
  r.h.al=mode;
  int86(0x10, &r, &r);
}

/**
* Set 80x50 TEXT mode
*/
void set80x50mode()
{
	regs16_t r;

	/* Set 80x25 mode */
	io_set_video_mode(0x03);

	/* Set 8x8 font */
	r.h.ah=0x11;
	r.h.al=0x12;
	r.h.bl=0x00;
	int86(0x10, &r, &r);

	/* Update BDA */
	lwriteb(0x40, 0x84, 50);
	lwriteb(0x40, 0x4a, 49);

	/* Update global VARs */
	screen_width=80;
	screen_height=50;
}

/**
* Set 320x200x256 VGA mode
*/
void set320x200mode()
{
  io_set_video_mode(0x13);
  screen_width=320;
  screen_height=200;
  video_mode=2;
}


/**
* Init PIT
*/
void init_pit(uint freq)
{
  if(freq==0) return;
  io_init_pit(1193180/freq);
}

/**
* Initialize disk
*/
void init_disk(uchar drive)
{
  io_init_disk(drive);
  switch(drive) {
    case 0x00: {
      disk.label[0]='f';
      disk.label[1]='d';
      disk.label[2]='0';
      disk.label[3]=0;
    } break;
    case 0x01: {
      disk.label[0]='f';
      disk.label[1]='d';
      disk.label[2]='1';
      disk.label[3]=0;
    } break;
    case 0x80: {
      disk.label[0]='h';
      disk.label[1]='d';
      disk.label[2]='0';
      disk.label[3]=0;
    } break;
    case 0x81: {
      disk.label[0]='h';
      disk.label[1]='d';
      disk.label[2]='1';
      disk.label[3]=0;
    } break;
    default: {
      disk.label[0]='u';
      disk.label[1]='n';
      disk.label[2]='k';
      disk.label[3]=0;
    } break;
  }
}

/**
* Kernel main function
*/
void kmain()
{
  int i;
  pic_remap();
  init_pit(PIT);

  #if TEXT_MODE==2
    set80x50mode();
  #else
  	#if TEXT_MODE!=1
  	  #error "TEXT MODE is invalid!"
  	#endif
  #endif
  clear_screen(0x07);
  puts("Initializing DISK...\n");
  if(init_disk(drive))
    puts("\033[37m[\033[31mERROR\033[37m]: Failed to initialize disk\n");
  else
    kputsf("\033[37m[\033[32m\033[5mOK\033[25m\033[37m]: Initialized DISK\n");
  if(bfx_mount())
    kputsf("\033[37m[\033[31m\033[5mERROR\033[25m\033[37m]: Failed to mount BFX in %s\n", disk.label);
  else
    kputsf("\033[37m[\033[32m\033[5mOK\033[25m\033[37m]: Mounted succefully BFX in %s\n", disk.label);

  /* Install syscall */
  setvect(0x32, sys_isr);

  clear_screen(0x07);
  exec("/bin/shell");
  puts("Halted\n");
  for(;;);
}
