/**
 * utils.c
 * Created by Matheus Leme Da Silva
 */
#include "utils.h"
#include "console.h"
#include "fs.h"
#include "kernel.h"
#include "types.h"
#include "video.h"
#include "x86.h"
#include "errno.h"

int errno;

/**
 * Convert number to string
 */
char *
numtostr (ulong val, uchar *out, ushort base, bool neg)
{
	char tmp[32];
	char *digits = "0123456789abcdef";
	int i = 0, j = 0;
	if (val == 0) {
		out[0] = '0';
		out[1] = '\0';
		return;
	}
	while (val && i < 31) {
		tmp[i++] = digits[val % base];
		val /= base;
	}
	if (neg)
		tmp[i++] = '-';
	while (i--)
		out[j++] = tmp[i];
	out[j] = 0;
}

/**
 * Compare strings
 */
uint
streq (uchar *a, uchar *b)
{
	while (*a && *b) {
		if (*a != *b)
			return 0;
		a++;
		b++;
	}
	return *a == *b;
}

/**
 * Compare n bytes in strings
 */
uint
strneq (uchar *a, uchar *b, uint n)
{
	while (n && *a && (*a == *b)) {
		a++;
		b++;
		n--;
	}
	if (n == 0)
		return 1;
	return (*a == *b);
}

/**
 * Return len of string
 */
uint
strlen (uchar *s)
{
	int size = 0;
	while (*s++)
		size++;
	return size;
}

/**
 * Interrupt
 */
void
int86 (uchar intnum, regs16_t *in, regs16_t *out)
{
	switch (intnum) {
	case 0x10: {
		setregs (in);
#asm
		int 0x10
#endasm
	} break;
	case 0x11: {
		setregs (in);
#asm
		int 0x11
#endasm
	} break;
	case 0x12: {
		setregs (in);
#asm
		int 0x12
#endasm
	} break;
	case 0x13: {
		setregs (in);
#asm
		int 0x13
#endasm
	} break;
	case 0x14: {
		setregs (in);
#asm
		int 0x14
#endasm
	} break;
	case 0x15: {
		setregs (in);
#asm
		int 0x15
#endasm
	} break;
	default: {
		puts ("Error: unknown interrupt!\n");
	} break;
	}
	getregs (out);
}

/**
 * Dump a buffer
 */
void
hexdump (void *data, uint size)
{
	uchar *buffer = (uchar *)data;
	uint i, j;
	uint bytes_per_line = (screen_width - 10) / 4;
	if (bytes_per_line == 0)
		bytes_per_line = 1;
	kputsf ("%-4s | %-*s | %-*s\n", "OFF", bytes_per_line * 3, "HEX", bytes_per_line, "ASCII");
	for (i = 0; i < screen_width - 2; i++) {
		putc ('-');
	}
	putc ('\n');
	for (i = 0; i < size; i += bytes_per_line) {
		kputsf ("%{0}04x |", i);
		for (j = 0; j < bytes_per_line; j++) {
			if (i + j < size)
				kputsf (" %{0}02x", buffer[i + j]);
			else
				puts ("   ");
		}
		puts ("  | ");
		for (j = 0; j < bytes_per_line; j++) {
			if (i + j < size) {
				uchar c = buffer[i + j];
				if (c >= 32 && c <= 126)
					putc (c);
				else
					putc ('.');
			} else {
				putc (' ');
			}
		}
		puts ("\n");
	}
}

/**
 * Find char in string
 */
char *
strchr (uchar *s, int c)
{
	while (*s) {
		if (*s == (uchar)c)
			return (uchar *)s;
		s++;
	}
	return NULL;
}

/**
 * Find string in string
 */
char *
strstr (uchar *haystack, uchar *needle)
{
	if (!*needle)
		return (uchar *)haystack;

	while (*haystack) {
		uchar *h = haystack;
		uchar *n = needle;

		while (*h && *n && *h == *n) {
			h++;
			n++;
		}

		if (!*n)
			return (uchar *)haystack;
		haystack++;
	}
}

/**
 * Copy string
 */
void
strcpy (uchar *dst, uchar *src)
{
	while (*src)
		*dst++ = *src++;
	*dst = 0;
}

/**
 * Copy n bytes string
 */
void
strncpy (uchar *dst, uchar *src, uint n)
{
	int i;
	for (i = 0; i < n && src[i] != 0; i++) {
		dst[i] = src[i];
	}
	dst[i] = 0;
}

/**
 * Concat string
 */
void
strcat (uchar *dst, uchar *src)
{
	while (*dst)
		dst++;
	while (*src)
		*dst++ = *src++;
	*dst = 0;
}

/**
 * Find last occurrence of a character in a string
 */
char *
strrchr (uchar *s, int c)
{
	uchar *last = NULL;
	while (*s) {
		if (*s == (uchar)c)
			last = s;
		s++;
	}
	return (char *)last;
}

/**
 * Fill buffer with a byte value
 * Set 'size' bytes at 'ptr' to 'val'
 */
void
memset (void *ptr, uchar val, uint size)
{
	uchar *p = (uchar *)ptr;
	uint i;
	for (i = 0; i < size; i++)
		p[i] = val;
}

/**
 * Copy bytes
 */
void *
memcpy (void *dst, void *src, uint size)
{
	uchar *s = (uchar *)src;
	uchar *d = (uchar *)dst;
	int i;
	for (i = 0; i < size; i++) {
		d[i] = s[i];
	}
	return dst;
}

/**
 * Move long memory
 */
void
lmemmove (ushort dst_seg, ushort dst_off, ushort src_seg, ushort src_off, ushort size)
{
	int i;
	if (dst_seg < src_seg || (dst_seg == src_seg && dst_off < src_off)) {
		for (i = 0; i < size; i++) {
			uchar b = lreadb (src_seg, src_off + i);
			lwriteb (dst_seg, dst_off + i, b);
		}
	} else {
		for (i = size - 1; i >= 0; i--) {
			uchar b = lreadb (src_seg, src_off + i);
			lwriteb (dst_seg, dst_off + i, b);
		}
	}
}

ulong
getms ()
{
	return ticks * (1000 / PIT);
}

void
sleep (ushort ms)
{
	ulong start = getms ();
	while ((getms () - start) < ms)
		;
}

/**
 * Exec file
 */
int
exec (uchar *path)
{
	int result;
	result = bfx_readfile (path, USERSEG, USEROFF + 512, MODE_EXEC, 0);
	if (result < 0)
		return result;

	lcall ();
	return 0;
}

/**
 * Copy long memory
 * Copy seg:off to buffer
 */
void
lmemcpy (void *buf, ushort seg, ushort off, uint size)
{
	int i;
	uchar *out = (uchar *)buf;
	for (i = 0; i < size; i++) {
		out[i] = lreadb (seg, off + i);
	}
}

/**
 * Copy long memory
 * Copy buffer to seg:off
 */
void
lrmemcpy (ushort seg, ushort off, void *buf, uint size)
{
	int i;
	uchar *in = (uchar *)buf;
#ifdef DEBUG
	printf ("Writing in %{0}4x:%{0}4x:\n", seg, off);
#endif
	for (i = 0; i < size; i++) {
		lwriteb (seg, off + i, in[i]);
#ifdef DEBUG
		putc (in[i]);
#endif
	}
#ifdef DEBUG
	printf ("done.\n");
#endif
}
