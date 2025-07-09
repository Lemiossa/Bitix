/**
 * ls.c
 * Created by Matheus Leme Da Silva
 */
#include <userlib.h>

#define MAX_PATHL 512
#define MAX_CWD 384

void
format_mode (uchar mode, uchar *out)
{
	if (mode & 0x80)
		out[0] = 'd';
	else
		out[0] = '-';

	out[1] = (mode & 0x04) ? 'r' : '-';
	out[2] = (mode & 0x02) ? 'w' : '-';
	out[3] = (mode & 0x01) ? 'x' : '-';
	out[4] = 0;
}

void
format_date (ushort date, uchar *out)
{
	ulong year = ((date >> 9) & 0x7f) + 2000;
	ulong month = (date >> 5) & 0x0f;
	ulong day = date & 0x1f;

	uchar stryear[5];
	uchar strmonth[3];
	uchar strday[3];

	numtostr (year, stryear, 10, false);
	numtostr (month, strmonth, 10, false);
	numtostr (day, strday, 10, false);

	memset (out, 0, 16);

	if (day < 10)
		strcat (out, "0");
	strcat (out, strday);
	strcat (out, "/");
	if (month < 10)
		strcat (out, "0");
	strcat (out, strmonth);
	strcat (out, "/");
	strcat (out, stryear);
}

int
main (int argc, char *argv[])
{
	uchar fullpath[MAX_PATHL];
	uchar cwd[MAX_CWD];
	uchar *path;
	int count;
	ushort dir_idx;
	ushort pos;
	int i;
	bfx_inode_t dir_inode;
	bool long_mode = 0;

	for (i = 1; i < argc; i++) {

		if (streq (argv[i], "-l")) {
			long_mode = true;
		} else {
			path = (uchar *)argv[i];
		}
	}

	if (path) {
		if (path[0] == '/') {
			strcpy (fullpath, path);
		} else {
			int len;
			if (getcwd (cwd, MAX_CWD) <= 0) {
				printf ("%s: Failed to get current directory\n", argv[0]);
				return 1;
			}
			strcpy (fullpath, cwd);
			len = strlen (fullpath);
			if (len > 0 && fullpath[len - 1] != '/')
				strcat (fullpath, "/");

			strcat (fullpath, path);
		}
	} else {
		if (getcwd (fullpath, MAX_PATHL) <= 0) {
			printf ("%s: Failed to get current directory\n", argv[0]);
			return 1;
		}
	}

	dir_idx = resolve_path (fullpath);
	if (dir_idx == 0) {
		puts ("ls: path not found\n");
		return 1;
	}

	if (read_inode (dir_idx, &dir_inode)) {
		puts ("ls: error reading inode\n");
		return 1;
	}

	if (!(dir_inode.mode & 0x80)) {
		puts ("ls: it is not a directory\n");
		return 1;
	}

	pos = 0;
	count = 0;
	while (pos < dir_inode.size) {
		ushort block_idx = dir_inode.start + pos / BLOCK;
		ushort offset = pos % BLOCK;
		ushort child_idx = 0;
		bfx_inode_t child;

		if (block_idx >= sb.total_blocks)
			break;
		if (readblock (block_idx, tmpbuf))
			break;

		child_idx = *((ushort *)(tmpbuf + offset));
		if (child_idx == 0)
			break;
		if (read_inode (child_idx, &child))
			break;

		if (long_mode) {
			uchar mode[5];
			uchar date[16];
			format_mode (child.mode, mode);
			format_date (child.created, date);
			if (IS_DIR (child.mode)) {
				printf ("%-4s DIR %-10s \033[34m%s\033[0m\n", mode, date, child.name);
			} else {
				if (child.size > 1024) {
					if (HAS_EXEC (child.mode))
						printf ("%-4 %4u KiB %-10s \033[32m%s\033[0m\n", mode, child.size / 1024, date, child.name);
					else
						printf ("%-4s %4u KiB %-10s %s\n", mode, child.size / 1024, date, child.name);
				} else {
					if (HAS_EXEC (child.mode))
						printf ("%-4s %4u   B %-10s \033[32m%s\033[0m\n", mode, child.size, date, child.name);
					else
						printf ("%-4s %4u   B %-10s %s\n", mode, child.size, date, child.name);
				}
			}
		} else {
			if (IS_DIR (child.mode)) {
				printf ("\033[34m%s  \033[0m", child.name);
			} else {
				if (HAS_EXEC (child.mode))
					printf ("\033[32m%s  \033[0m", child.name);
				else
					printf ("%s  ", child.name);
			}
			count++;
			if (count % 6 == 0)
				putc ('\n');
		}
		pos += sizeof (ushort);
	}
	if (!long_mode)
		printf ("\n");
	return 0;
}
