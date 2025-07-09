/**
 * kernel.c
 * Created by Matheus Leme Da Silva
 */

#include "kernel.h"
#include "console.h"
#include "types.h"
#include "utils.h"
#include "x86.h"

/**
 * Remap PIC
 */
void
pic_remap ()
{
	outportb (0x20, 0x11);
	outportb (0xa0, 0x11);
	outportb (0x21, 0x20);
	outportb (0xa1, 0x28);
	outportb (0x21, 0x04);
	outportb (0xa1, 0x02);
	outportb (0x21, 0x01);
	outportb (0xa1, 0x01);
}

/**
 * SERIAL
 */
#define COM1 0x3F8
void
serial_init ()
{
	outportb (COM1 + 1, 0x00); // Disable all interrupts
	outportb (COM1 + 3, 0x80); // Enable DLAB (set baud rate divisor)
	outportb (COM1 + 0, 0x03); // Divisor low byte (38400 baud)
	outportb (COM1 + 1, 0x00); // Divisor high byte
	outportb (COM1 + 3, 0x03); // 8 bits, no parity, one stop bit
	outportb (COM1 + 2, 0xC7); // Enable FIFO, clear them, with 14-byte threshold
	outportb (COM1 + 4, 0x0B); // IRQs enabled, RTS/DSR set
}

int
serial_is_transmit_ready ()
{
	return inportb (COM1 + 5) & 0x20;
}

void
serial_putc (uchar c)
{
	while (!serial_is_transmit_ready ())
		;
	outportb (COM1, c);
}

/**
 * Init PIT
 */
void
init_pit (uint freq)
{
	if (freq == 0)
		return;
	io_init_pit (1193180 / freq);
}

/**
 * Kernel main function
 */
void
kmain ()
{
	int file;
	pic_remap ();
	init_pit (PIT);
	serial_init ();

#if TEXT_MODE == 1
	set40x25mode ();
#elif TEXT_MODE == 2
	set80x25mode ();
#elif TEXT_MODE == 3
	set80x50mode ();
#else
#error Invalid TEXT Mode
#endif

	clear_screen (0x07);
	puts ("Initializing DISK...\n");
	if (init_disk (drive))
		puts ("\033[37m[\033[31mERROR\033[37m]: Failed to initialize disk\n");
	else
		kputsf ("\033[37m[\033[32m\033[5mOK\033[25m\033[37m]: Initialized DISK\n");
	if (bfx_mount ())
		kputsf ("\033[37m[\033[31m\033[5mERROR\033[25m\033[37m]: Failed to mount BFX in %s\n", disk.label);
	else
		kputsf ("\033[37m[\033[32m\033[5mOK\033[25m\033[37m]: Mounted succefully BFX in %s\n", disk.label);

	/* Install syscall */
	setvect (0x80, sys_isr);

	init_fd ();

	shell ();
	puts ("Halted\n");
	for (;;)
		;
}

extern uchar cwd[384];
int
syscall (ushort id, ushort arg1, ushort arg2, ushort arg3, ushort arg4, ushort arg5)
{
	switch (id) {
	case SYSCALL_PUTC: {
		putc ((uchar)arg1);
	} break;
	case SYSCALL_PUTS: {
		ushort seg = USERSEG;
		ushort off = arg1;
		while (1) {
			uchar ch = lreadb (seg, off++);
			if (ch == 0)
				break;
			putc (ch);
		}
	} break;
	case SYSCALL_SET_CURSOR: {
		setcursor (arg1, arg2);
	} break;
	case SYSCALL_CLEAR_SCREEN: {
		clear_screen (arg1);
	} break;
	case SYSCALL_SET_VID: {
		switch (arg1) {
		case 1: {
			set40x25mode ();
		} break;
		case 2: {
			set80x25mode ();
		} break;
		case 3: {
			set80x50mode ();
		} break;
		case 4: {
			set320x200mode ();
		} break;
		default: {
			kputsf ("Unknown video mode: %u\n", arg1);
		} break;
		}
	} break;
	case SYSCALL_READBLOCK: {
		freadblock (arg1, arg2, arg3);
	} break;
	case SYSCALL_WRITEBLOCK: {
		fwriteblock (arg1, arg2, arg3);
	} break;
	case SYSCALL_SLEEP: {
		sleep (arg1);
	} break;
	case SYSCALL_GET_CURSOR_POSITION: {
		ushort seg = USERSEG;
		ushort x = arg1;
		ushort y = arg2;

		lwriteb (seg, x, cursor_x);
		lwriteb (seg, y, cursor_y);
	} break;
	case SYSCALL_DRAW_PIXEL: {
		putpixel (arg1, arg2, arg3);
	} break;
	case SYSCALL_GETCWD: {
		ushort seg = USERSEG;
		ushort off = arg1;
		int i = 0;
		while (cwd[i]) {
			lwriteb (seg, off++, cwd[i++]);
		}
		lwriteb (seg, off, 0);
	} break;
	case SYSCALL_PUTXY_CT: {
		uchar x = (uchar)arg1;
		uchar y = (uchar)arg2;
		uchar c = (uchar)arg3;
		uchar attr = (uchar)arg4;

		putxy (x, y, c, attr);
	} break;
	case SYSCALL_GET_KEY: {
		ushort seg = USERSEG;
		ushort off = arg1;
		lwritew (seg, off, getkey ());
	} break;
	default: {
		kputsf ("Unknown syscall 0x%x\n", id);
	} break;
	}
}
