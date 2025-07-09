/**
 * userlib.c
 * Created by Matheus Leme da Silva
 */
#include "types.h"
#include "userlib.h"
#include "kernel.h"

int shift_pressed = 0;
int caps_lock = 0;
int ctrl_pressed = 0;

uchar tmpbuf[BLOCK];
bfx_super_t sb;

/**
 * Setup superblock variable
 */
void
setup_superblock ()
{
	readblock (BFX_SUPER_SEC, tmpbuf);
	memcpy (&sb, tmpbuf, sizeof (bfx_super_t));
}

/**
 * Make args
 */
void
__mkargv ()
{
	int argc = *(int *)0x0000;
	char **argv = (char **)0x0002;
	setup_superblock ();
	main (argc, argv);
}

/**
 * Put char in the screen
 */
void
putc (uchar c)
{
	syscall (SYSCALL_PUTC, c, 0, 0, 0, 0);
}

/**
 * Put string in the screen
 */
void
puts (uchar *s)
{
	syscall (SYSCALL_PUTS, s, 0, 0, 0, 0);
}

/**
 * Set cursor position in the screen
 */
void
setcursor (uint x, uint y)
{
	syscall (SYSCALL_SET_CURSOR, x, y, 0, 0, 0);
}

/**
 * Clear the screen with attrubute
 */
void
clear_screen (uchar attr)
{
	syscall (SYSCALL_CLEAR_SCREEN, attr, 0, 0, 0, 0);
}

/**
 * Set video mode
 */
void
setvid (int mode)
{
	syscall (SYSCALL_SET_VID, mode, 0, 0, 0, 0);
}

/**
 * Read disk block
 */
int
readblock (ushort lba, uchar *buf)
{
	ushort seg = FILESEG + 0x1000, off = 0x0000;
	syscall (SYSCALL_READBLOCK, lba, seg, off, 0, 0);
	lmemcpy (buf, FILESEG + 0x1000, off, BLOCK);
	return 0;
}

/**
 * Write disk block
 */
int
writeblock (ushort lba, uchar *buf)
{
	ushort seg = FILESEG + 0x1000, off = 0x0000;
	syscall (SYSCALL_WRITEBLOCK, lba, seg, off, 0, 0);
	lrmemcpy (FILESEG + 0x1000, off, buf, BLOCK);
	return 0;
}

/**
 * Get current MS
 */
void
sleep (uint ms)
{
	syscall (SYSCALL_SLEEP, ms, 0, 0, 0, 0);
}

void
get_cursor_pos (uchar *x, uchar *y)
{
	syscall (SYSCALL_GET_CURSOR_POSITION, (ushort)x, (ushort)y, 0, 0, 0);
}

/**
 * Draw pixel in the screen
 */
void
putpixel (uint x, uint y, uchar color)
{
	syscall (SYSCALL_DRAW_PIXEL, x, y, (uint)color, 0, 0);
}

/**
 * Get CWD from shell
 */
int
getcwd (uchar *buf, int max_len)
{
	syscall (SYSCALL_GETCWD, (ushort)buf, 0, 0, 0, 0);
	return strlen (buf);
}

/**
 * Put char in x and y with attr
 */
void
putxy (uchar x, uchar y, uchar chr, uchar attr)
{
	syscall (SYSCALL_PUTXY_CT, x, y, chr, attr, 0);
}

/**
 * Get KEY
 */
uint
getkey ()
{
	uint key;
	syscall (SYSCALL_GET_KEY, (ushort)&key, 0, 0);
}

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
 * Put string with padding
 */
void
putsp (uchar *s, int width, uchar pad_char, bool neg)
{
	int len = 0;
	int pad;
	while (s[len])
		len++;
	pad = (width > 0) ? (width - len) : (-width - len);
	if (pad < 0)
		pad = 0;
	if (!neg) {
		while (pad-- > 0)
			putc (pad_char);
		puts (s);
	} else {
		puts (s);
		while (pad-- > 0)
			putc (pad_char);
	}
}

/**
 * Put string formatted
 */
#define __max_buf_size 32
void
printf (uchar *format, ...)
{
	uint *arg = (uint *)(&format + 1);
	while (*format) {
		if (*format == '%') {
			uchar buf[__max_buf_size];
			long val;
			ulong uval;
			long neg;
			bool is_long = false;
			uint base = 10;
			int width = 0;
			bool width_neg = false;
			uchar pad_char = ' ';

			format++;

			if (*format == '{' && format[2] == '}') {
				pad_char = format[1];
				format += 3;
			}

			if (*format == '-') {
				width_neg = true;
				width = -width;
				format++;
			}

			if (*format == '*') {
				width = *((uint *)arg++);
				format++;
			} else {
				while (*format >= '0' && *format <= '9') {
					width = width * 10 + (*format - '0');
					format++;
				}
			}

			if (*format == 'l') {
				val = *((ulong *)arg);
				arg += 2;
				is_long = true;
				format++;
			} else {
				val = *((uint *)arg++);
				is_long = false;
			}
			uval = val;
			neg = ((val & (is_long ? 0x80000000 : 0x8000)) ? true : false);
			if (neg) {
				if (is_long) {
					val = (~val) + 1;
				} else {
					val = (ulong)(-(slong)val) + 1;
				}
			}

			if (*format == 'd' || *format == 'i') {
				base = 10;
			} else if (*format == 'u') {
				base = 10;
				neg = false;
				val = uval;
			} else if (*format == 'x') {
				val = uval;
				neg = false;
				base = 16;
			} else if (*format == 'b') {
				base = 2;
			} else if (*format == 'o') {
				base = 8;
			} else if (*format == '%') {
				puts ("%");
				format++;
				continue;
			} else if (*format == 's') {
				uchar *str = val;
				if (str) {
					if (width)
						putsp (str, width, ' ', width_neg);
					else
						puts (str);
				}
				format++;
				continue;
			} else {
				format++;
				continue;
			}
			format++;
			numtostr (val, buf, base, neg);
			if (width)
				putsp (buf, width, pad_char, width_neg);
			else
				puts (buf);
		} else {
			putc (*format++);
		}
	}
}

/**
 * Read a inode with index
 */
int
read_inode (ushort idx, bfx_inode_t *out)
{
	ushort block, offset;
	uchar *inode_ptr;
	int i;

	if (idx == 0)
		return ENOTFOUND;

	block = sb.inode_start + idx / INPB;
	offset = idx % INPB;

	if (block >= sb.total_blocks) {
		printf ("read_inode: Invalid block: %u>%u\n", block, sb.total_blocks);
		return EIO;
	}

	if (readblock (block, tmpbuf)) {
		printf ("read_inode: Failed to read block %u\n", block);
		return EIO;
	}
	inode_ptr = tmpbuf + offset * sizeof (bfx_inode_t);
	memcpy (out, inode_ptr, sizeof (bfx_inode_t));

	return 0;
}

/**
 * Compare BFX filenames
 */
int
name_match (bfx_inode_t *inode, uchar *name)
{
	int i;
	for (i = 0; i < MAX_NAME; i++) {
		if (name[i] == 0 && inode->name[i] == 0)
			return 1;
		if (name[i] != inode->name[i])
			return 0;
	}
	return 0;
}

/**
 * Parse PATH
 */
int
parse_path (uchar *path, uchar components[MAX_PATH][MAX_NAME])
{
	int comp_count = 0;
	int pos = 0;
	int cpos = 0;
	if (!path || path[0] != '/')
		return -1;
	pos++;
	while (path[pos] && comp_count < MAX_PATH) {
		if (path[pos] == '/') {
			if (cpos > 0) {
				components[comp_count][cpos] = 0;
				comp_count++;
				cpos = 0;
			}
			pos++;
			continue;
		}
		if (cpos < MAX_NAME - 1) {
			components[comp_count][cpos++] = path[pos++];
		} else {
			return -1;
		}
	}
	if (cpos > 0) {
		components[comp_count][cpos] = 0;
		comp_count++;
	}
	return comp_count;
}

/**
 * Find inode in directory
 */
ushort
find_inode_in_dir (ushort dir_idx, uchar *name)
{
	bfx_inode_t dir;
	ushort pos = 0;
	if (read_inode (dir_idx, &dir))
		return 0;
	if (!(dir.mode & 0x80))
		return 0;
	while (pos < dir.size) {
		ushort block_idx = dir.start + pos / BLOCK;
		ushort offset = pos % BLOCK;
		ushort child_idx = 0;
		bfx_inode_t child;
		if (block_idx >= sb.total_blocks)
			return 0;
		if (readblock (block_idx, tmpbuf))
			return 0;
		child_idx = *((ushort *)(tmpbuf + offset));
		if (child_idx == 0)
			return 0;
		if (read_inode (child_idx, &child))
			return 0;
		if (name_match (&child, name))
			return child_idx;
		pos += sizeof (ushort);
	}
	return 0;
}

/**
 * Resolve path
 */
ushort
resolve_path (uchar *path)
{
	uchar components[MAX_PATH][MAX_NAME];
	int count = parse_path (path, components);
	ushort current;
	int i;
	if (count < 0)
		return 0;
	current = sb.root_inode;
	for (i = 0; i < count; i++) {
		current = find_inode_in_dir (current, components[i]);
		if (current == 0)
			return 0;
	}
	return current;
}

/**
 * Load file in BFX
 */
int
bfx_readfile (uchar *path, ushort seg, ushort off, uchar need, uchar forbid)
{
	ushort idx;
	bfx_inode_t file;
	ushort size;
	ushort start_block;
	ushort current_off;
	ushort i;
	if (path[0] != '/')
		return EINVPATH;

	idx = resolve_path (path);

	if (idx == 0)
		return ENOTFOUND;

	if (read_inode (idx, &file))
		return EIO;

	if (IS_DIR (file.mode))
		return ENOTFILE;

	if ((file.mode & MODE_MASK & need) != (need & MODE_MASK))
		return EPERMDEN;

	if (file.mode & forbid)
		return EPERMDEN;

	size = file.size;
	start_block = file.start;

	for (i = 0; i < file.size_blocks; i++) {
		if (start_block + i >= sb.total_blocks)
			return EIO;

		if (readblock (start_block + i, tmpbuf))
			return EIO;

		lrmemcpy (seg, current_off, tmpbuf, BLOCK);
		current_off += BLOCK;
	}
#ifdef DEBUG
	printseg (seg, off, size);
#endif
	return size;
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
 * Print memory
 */
void
printseg (ushort seg, ushort off, uint size)
{
	int i = 0;
	printf ("Reading %{0}4x:%{0}4x:\n", seg, off);
	for (; i < size; i++) {
		putc (lreadb (seg, off + i));
	}
	printf ("done. %{0}4x:%{0}4x\n", seg, off + i);
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

#define MAX_LINE 256

int
readline (uchar *buffer, int max_len)
{
	int pos = 0;
	uint len = 0;
	while (1) {
		uint key = getkey ();
		len = strlen (buffer);
		if (key == 0)
			continue;
		if (key == '\r') {
			putc ('\n');
			buffer[pos] = 0;
			return pos;
		} else if (key == '\b') {
			if (pos > 0) {
				pos--;
				putc ('\b');
				putc (' ');
				putc ('\b');
			}
		} else if (key == KEY_LEFT) {
			if (pos > 0) {
				ushort *x;
				ushort *y;
				get_cursor_pos (&x, &y);
				setcursor (x - 1, y);
				pos--;
			}
		} else if (key == KEY_RIGHT) {
			if (pos < len) {
				ushort *x;
				ushort *y;
				get_cursor_pos (&x, &y);
				setcursor (x + 1, y);
				pos++;
			}
		} else {
			if (pos < max_len - 1) {
				if (key >= 32 && key <= 126) {
					putc ((uchar)key);
					buffer[pos++] = (uchar)key;
				}
			}
		}
	}
}
