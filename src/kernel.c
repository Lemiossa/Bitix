/**
* kernel.c
* Created by Matheus Leme Da Silva
*/

#include <types.h>
#include <x86.h>
#include <utils.h>

uchar current_attr=0x07;
uint screen_width=80;
uint screen_height=25;
uchar video_mode=1;

uchar buf[512];

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

  kputsf("Reading '%s'...\n", "/boot/boot.cfg");
  if(bfx_readfile("/boot/boot.cfg", 0x1800, 0x0000)<0)
    kputsf("\033[31mFailed to read file '%s'\033[0m\n", "/boot/boot.cfg");
  else
    kputsf("\033[32mReaded file '%s'\033[0m\n", "/boot/boot.cfg");

  for(i=0;i<512;i++) {
    buf[i]=lread(0x1800, 0x0000+i);
  }

  kputsf("File content:\n");
  puts(buf);

  for(;;);
}
