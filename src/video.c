/**
 * video.c
 * Created by Matheus Leme Da Silva
 */
#include "console.h"
#include "x86.h"

uint screen_width = 80;
uint screen_height = 25;
uchar video_mode = 1;

/**
 * Set VGA VIDEO/TEXT mode
 */
void
set_video_mode (uchar mode)
{
	regs16_t r;
	r.h.ah = 0x00;
	r.h.al = mode;
	int86 (0x10, &r, &r);
}

/**
 * Set 40x25 TEXT mode
 */
void
set40x25mode ()
{
	regs16_t r;

	/* Set 40x25 mode */
	set_video_mode (0x00);

	/* Update BDA */
	lwriteb (0x40, 0x84, 24);
	lwriteb (0x40, 0x4a, 16);

	/* Update global VARs */
	screen_width = 40;
	screen_height = 25;
}

/**
 * Set 80x25 TEXT mode
 */
void
set80x25mode ()
{
	regs16_t r;

	/* Set 80x25 mode */
	set_video_mode (0x03);

	/* Update BDA */
	lwriteb (0x40, 0x84, 24);
	lwriteb (0x40, 0x4a, 16);

	/* Update global VARs */
	screen_width = 80;
	screen_height = 25;
}

/**
 * Set 80x50 TEXT mode
 */
void
set80x50mode ()
{
	regs16_t r;

	/* Set 80x25 mode */
	set_video_mode (0x03);

	/* Set 8x8 font */
	r.h.ah = 0x11;
	r.h.al = 0x12;
	r.h.bl = 0x00;
	int86 (0x10, &r, &r);

	/* Update BDA */
	lwriteb (0x40, 0x84, 49);
	lwriteb (0x40, 0x4a, 8);

	/* Update global VARs */
	screen_width = 80;
	screen_height = 50;
}

/**
 * Set 320x200x256 VGA mode
 */
void
set320x200mode ()
{
	set_video_mode (0x13);
	screen_width = 320;
	screen_height = 200;
	video_mode = 2;
}

/**
 * Set 640x480x256 VESA mode
 */
void
set640x480mode ()
{
	if (io_set_vesa_mode (0x4101) != 0x004f) {
		puts ("Failed to initialize VESA 640x480 MODE\n");
		return;
	}
	screen_width = 640;
	screen_height = 480;
	video_mode = 3;
}

/**
 * Put pixel in graphic mode
 */
ushort current_bank = 0xffff;
void
putpixel (uint x, uint y, uchar color)
{
	if (x >= screen_width || y >= screen_height)
		return;
	if (video_mode == 3) { /* video_mode=3 - 640x480x256 VESA mode */
		ulong offset = (ulong)y * screen_width + x;
		ulong bank_off, bank;

		bank = offset / 65536;
		bank_off = offset % 65536;
		if (bank != current_bank) {
			if (!io_set_vesa_bank (bank)) {
				puts ("Failed to set VESA bank\n"); // Print serial
				return;
			}
			current_bank = (ushort)bank;
		}
		lwriteb (0xa000, (ushort)bank_off, color);
	} else if (video_mode == 2) { /* video_mode=2 - 320x200x256 VGA mode */
		ushort offset = y * screen_width + x;
		lwriteb (0xa000, offset, color);
	} else if (video_mode == 1)
		puts ("ERROR: Attempted to draw a pixel in an incompatible mode\n");
	return;
}
