/**
* main.c
* Created by Matheus Leme Da Silva
*/
#include "types.h"
#include "x86.h"
#include "utils.h"

/* Segment and offset where the file will be loaded */
#define LSEG 0x0800
#define LOFF 0x0000

/* Configuration for boot menu */
#define MAX_ENTRIES 4
#define MAX_NAME_LEN 32
#define MAX_PATH_LEN 64
#define CFG_SEG 0x2000
#define CFG_OFF 0x0000

#define MENU_WIDTH 70
#define MENU_HEIGHT (15+MAX_ENTRIES+2)
#define MENU_X 5
#define MENU_Y 2

#define BOX_ATTR 0x07
#define SELECT_ATTR 0x70

typedef struct {
  uchar name[MAX_NAME_LEN];
  uchar path[MAX_PATH_LEN];
} entry_t;

entry_t entries[MAX_ENTRIES];
int entry_count=0;

/**
* Parse the loaded boot.cfg file in memory and fill the entries array
* Supports lines like:
*   name=path/to/entry
*   default=entry_name
*/
void parse_cfg_seg(u16 seg, u16 off)
{
  int i=0,line=0, c;
  int name_len, path_len;
  uchar temp_name[MAX_NAME_LEN];
  uchar temp_path[MAX_PATH_LEN];

  while(1) {
    name_len=0;
    path_len=0;

    /* Skip empty lines */
    while(1) {
      c=lread8(seg, off+i);
      if(c=='\n') { i++; continue; }
      if(c==0) goto end;
      break;
    }

    /* TODO: Implement default system */

    /* Read name until '=' */
    while(1) {
      c=lread8(seg, off+i++);
      if(c==0||c=='\n'||c=='=')break;
      if(name_len<MAX_NAME_LEN-1) temp_name[name_len++]=c;
    }
    temp_name[name_len]=0;

    /* Skip malformed lines */
    if(c!='=') {
      while(c!=0&&c!='\n')c=lread8(seg, off+i++);
      continue;
    }

    /* Read path */
    while(1) {
      c=lread8(seg, off+i++);
      if(c==0||c=='\n') break;
      if(path_len<MAX_PATH_LEN-1) temp_path[path_len++]=c;
    }
    temp_path[path_len]=0;

    /* Add new boot entry */
    if(line<MAX_ENTRIES&&name_len>0&&path_len>0) {
      int j;
      for(j=0;j<=name_len;j++)entries[line].name[j]=temp_name[j];
      for(j=0;j<=path_len;j++)entries[line].path[j]=temp_path[j];
      line++;
    }
  }
end:
  entry_count=line;
}

/**
* Draws the boot menu UI box and entries.
* Highlights the selected entry.
*/
void draw_box_menu(int sel)
{
  int x=MENU_X, y=MENU_Y;
  int w=MENU_WIDTH, h=MENU_HEIGHT;
  int i, j;
  uchar *title=" Bitix Bootloader ";
  uchar *help="[ENTER] boot   [ESC] cancel";

  /* Top border */
  putcat(201, x, y, BOX_ATTR);
  for(i=1;i<w-1;i++)putcat(205, x+i, y, BOX_ATTR);
  putcat(187, x+w-1, y, BOX_ATTR);

  /* Sides and content area */
  for(j=1;j<h-1;j++) {
    putcat(186, x, y+j, BOX_ATTR);
    for(i=1;i<w-1;i++)putcat(' ', x+i, y+j, BOX_ATTR);
    putcat(186, x+w-1, y+j, BOX_ATTR);
  }

  /* Bottom border */
  putcat(200, x, y+h-1, BOX_ATTR);
  for(i=1;i<w-1;i++) putcat(205, x+i, y+h-1, BOX_ATTR);
  putcat(188, x+w-1, y+h-1, BOX_ATTR);

  /* Title */
  for(i=0;title[i];i++)putcat(title[i], x+2+i, y, BOX_ATTR);
  for(i=0;i<entry_count&i<MAX_ENTRIES;i++) {
    int xpos=x+3;
    int ypos=y+2+i;
    uchar *text=entries[i].name;
    uchar attr=(i==sel)?SELECT_ATTR:BOX_ATTR;

    for(j=0;j<MENU_WIDTH-6;j++) {
      uchar c=text[j];
      if(!c)break;
      putcat(c, xpos+j, ypos, attr);
    }
  }

  /* Help line */
  for(i=0;help[i];i++)putcat(help[i], x+2+i, y+h-2, BOX_ATTR);
}

/**
* Show the boot menu and return the selected index.
* Handles keyboard navigation using getkey.
*/
int show_menu()
{
  int sel=0, prev=-1;
  int i, key;

  /* Menu loop */
  while(1) {
    if(sel!=prev) {
      draw_box_menu(sel);
      prev=sel;
    }
    key=getkey();
    if(key==KEY_UP&&sel>0) sel--;
    else if(key==KEY_DOWN&&sel<entry_count-1)sel++;
    else if(key=='\n')return sel;
    else if(key==KEY_ESC)return -1;
  }
}

/* Default kernel PATH */
uchar *filename="kernel.bin";

/**
* Main bootloader entry point
*/
void main()
{
  u32 read;
  int i, opt;
  io_set_video_mode(0x03);
  puts("Booting...\r\n");

  /* Initialize disk */
  if(io_init_disk(drive)) {
    puts("ERROR: Failed to initialize disk.\r\nHalted.");
    for(;;);
  }

  /* Mount BFX filesystem */
  if(bfx_mount()) {
    puts("ERROR: Failed to mount FS.\r\nHalted.");
    for(;;);
  }

  /* Load config file */
  read=bfx_readfile("boot.cfg", CFG_SEG, CFG_OFF);
  if(read<0) {
    kputsf("Failed to load '%s'\r\nHalted.");
    for(;;);
  }

  /* Parse entries */
  parse_cfg_seg(CFG_SEG, CFG_OFF);

  /* Abort if no valid entries */
  if(entry_count==0) {
    puts("No entries in boot.cfg\r\n");
    puts("Halted.\r\n");
    for(;;);
  }

  /* Show the menu if there is more than one entry */
  if(entry_count>1) {
    opt=show_menu();

    if(opt<0) {
      puts("Boot canceled.\r\n");
      for(;;);
    }
    /* Load selected file */
    filename=entries[opt].path;
  }
  kputsf("Loading '%s'...\r\n", filename);

  read=bfx_readfile(filename, LSEG, LOFF);
  if(read<0) {
    kputsf("Failed to boot '%s'\r\n", filename);
    for(;;);
  }

  /* Pass drive via BX and jump to file */
  setdx((u16)drive);
  ljump(LSEG, LOFF);
}
