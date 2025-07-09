/**
 * fs.c
 * Created by Matheus Leme Da Silva
 */
#include "fs.h"
#include "disk.h"
#include "types.h"
#include "utils.h"
#include "x86.h"

uchar tmpbuf[TMP_BUF_SIZE];

/**
 * Mount BFX filesystem
 */
int
bfx_mount ()
{
	if (readblock (BFX_SUPER_SEC, tmpbuf))
		return EIO;
	memcpy ((void *)&sb, tmpbuf, sizeof (bfx_super_t));
	if (sb.magic[0] != 'B' || sb.magic[1] != 'F' || sb.magic[2] != 'X') {
		kputsf ("mount: magic is invalid\r\n");
		return EIO;
	}
	return 0;
}

/**
 * Read a inode with index
 */
int
read_inode (ushort idx, bfx_inode_t *out)
{
	ushort block, offset;
	char *inode_ptr;

	if (idx == 0)
		return ENOTFOUND;

	block = sb.inode_start + idx / INPB;
	offset = idx % INPB;

	if (block >= sb.total_blocks)
		return EIO;

	if (readblock (block, tmpbuf))
		return EIO;
	inode_ptr = tmpbuf + offset * sizeof (bfx_inode_t);
	memcpy (out, inode_ptr, sizeof (bfx_inode_t));

	return 0;
}

/**
 * Compare BFX filenames
 */
int
name_match (bfx_inode_t *inode, char *name)
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
parse_path (char *path, char components[MAX_PATH][MAX_NAME])
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
find_inode_in_dir (ushort dir_idx, char *name)
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
resolve_path (char *path)
{
	char components[MAX_PATH][MAX_NAME];
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
	ushort bytes_left;
	ushort current_seg, current_off;
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
	bytes_left = size;
	current_seg = seg;
	current_off = off;

	for (i = 0; i < (size + BLOCK - 1) / BLOCK; i++) {
		ushort to_copy, j;
		if (start_block + i >= sb.total_blocks)
			return EIO;
		if (readblock (start_block + i, tmpbuf))
			return EIO;
		to_copy = bytes_left > BLOCK ? BLOCK : bytes_left;
		for (j = 0; j < to_copy; j++) {
			lwriteb (current_seg, current_off + j, tmpbuf[j]);
		}
		current_off += BLOCK;
		bytes_left -= to_copy;
	}
	return size;
}

fd_t fd_table[MAX_FILES_OPENED];

void
init_fd ()
{
	memset (fd_table, 0, sizeof (fd_table));
	kputsf ("FD system initialized %u bytes\n", sizeof (fd_table));
}
int open (char *path);
int close (int fd);
int read (int fd, uptr_t buf, int count);
