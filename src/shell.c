/**
 * Shell.c
 * Created by Matheus Leme Da Silva
 */
#include "console.h"
#include "fs.h"
#include "kernel.h"
#include "types.h"
#include "utils.h"
#include "x86.h"

uchar cwd[384] = "/home";

#define MAX_ARGS 8
#define ARGV_BASE 0x0002
#define ARGS_SEG USERSEG

/**
 * Normalize PATH
 * Resolve . and ..
 */
static void
normalize_path (uchar *path, uchar *normalized)
{
	uchar parts[MAX_PATH][MAX_NAME];
	int count = parse_path ((uchar *)path, parts);
	int i, j = 0;
	uchar temp[MAX_PATH][MAX_NAME];
	int top, pos;

	if (count < 0) {
		normalized[0] = 0;
		return;
	}

	top = 0;
	for (i = 0; i < count; i++) {
		if (streq (parts[i], (uchar *)".")) {
			continue;
		} else if (streq (parts[i], (uchar *)"..")) {
			if (top > 0)
				top--;
		} else {
			for (j = 0; parts[i][j]; j++)
				temp[top][j] = parts[i][j];
			temp[top][j] = 0;
			top++;
		}
	}

	pos = 0;
	normalized[pos++] = '/';
	for (i = 0; i < top; i++) {
		for (j = 0; temp[i][j]; j++)
			normalized[pos++] = temp[i][j];
		if (i != top - 1)
			normalized[pos++] = '/';
	}

	if (pos == 1)
		normalized[pos] = 0;
	else
		normalized[pos] = 0;
}

/**
 * Setup argv
 */
int
setup_argv (int argc, char **argv)
{
	ushort seg = ARGS_SEG;
	ushort off = 0;
	ushort str_off = ARGV_BASE + argc * 2;
	int i;
	uchar buf[128];

	/** Clean **/
	for (i = 0; i < 256; i++) {
		lwritew (ARGS_SEG, 0x0000 + i, 0);
	}

	lwriteb (seg, 0x0000, argc);
	for (i = 0; i < argc; i++) {
		char *s = argv[i];
		lwritew (seg, ARGV_BASE + i * 2, str_off);
		while (*s)
			lwriteb (seg, str_off++, *s++);
		lwriteb (seg, str_off++, 0);
	}
	return 0;
}

/**
 * Execute command from PATH
 */
int
run_cmd (int argc, char **argv)
{
	uchar fullpath[384];
	uchar path[384];
	int res;
	ushort inode;
	bfx_inode_t inode_data;

	if (argv[0][0] == '/') {
		strcpy (fullpath, argv[0]);
	} else {
		strcpy (fullpath, "/bin/");
		strcat (fullpath, argv[0]);
	}

	normalize_path (fullpath, path);

	inode = resolve_path (path);
	if (inode == 0)
		return ENOTFOUND;

	if (read_inode (inode, &inode_data))
		return EIO;

	if (!IS_FILE (inode_data.mode))
		return ENOTFILE;

	res = bfx_readfile (path, USERSEG, USEROFF + 0x0200, MODE_EXEC, 0);
	if (res < 0)
		return res;

	setup_argv (argc, argv);
	return lcall (USERSEG, USEROFF + 0x0200);
}

/**
 * Set CWD
 */
int
set_cwd (uchar *path)
{
	uchar fullpath[384];
	uchar normpath[384];
	bfx_inode_t inode_data;
	ushort inode;
	int i;

	if (path[0] != '/') {
		int len = 0, j = 0;
		while (cwd[len]) {
			fullpath[len] = cwd[len];
			len++;
		}
		if (len > 0 && fullpath[len - 1] != '/') {
			fullpath[len++] = '/';
		}
		while (path[j]) {
			fullpath[len++] = path[j++];
		}
		fullpath[len] = 0;
	} else {
		int k = 0;
		while (path[k]) {
			fullpath[k] = path[k];
			k++;
		}
		fullpath[k] = 0;
	}
	normalize_path (fullpath, normpath);

	inode = resolve_path (normpath);
	if (inode == 0)
		return ENOTFOUND;

	if (read_inode (inode, &inode_data))
		return EIO;

	if (!IS_DIR (inode_data.mode))
		return ENOTFILE;

	i = 0;
	while (normpath[i] && i < sizeof (cwd) - 1) {
		cwd[i] = normpath[i];
		i++;
	}
	cwd[i] = 0;
	return 0;
}

/**
 * Mini shell
 */
void
shell ()
{
	uchar line[256];
	uchar *argv[MAX_ARGS];
	int argc;
	int i;

	clear_screen (0x07);

	puts ("Welcome to \033[5mBitix\033[25m shell!\n");
	puts ("Created by Matheus Leme Da Silva\n");
	for (;;) {
		uchar *tok;
		memset (line, 0, sizeof (line));
		for (i = 0; i < argc; i++) {
			memset (argv[i], 0, strlen (argv[i]));
		}
		kputsf ("\033[34m%s \033[32m$\033[0m ", cwd);
		readline (line, sizeof (line));
		if (strlen (line) == 0)
			continue;
		argc = 0;
		tok = line;
		while (*tok && argc < MAX_ARGS) {
			while (*tok == ' ')
				tok++;
			if (!*tok)
				break;
			argv[argc++] = tok;
			while (*tok && *tok != ' ')
				tok++;
			if (*tok)
				*tok++ = '\0';
		}
		if (argc == 0)
			continue;
		if (streq (argv[0], "exit")) {
			puts ("Bye!\n");
			return;
		} else if (streq (argv[0], "cd")) {
			int res;
			if (argc < 2) {
				kputsf ("%s: Missing operand\n", argv[0]);
				continue;
			}
			res = set_cwd (argv[1]);
			if (res == ENOTFOUND) {
				kputsf ("%s: %s: No such directory\n", argv[0], argv[1]);
			} else if (res == EIO) {
				kputsf ("%s: %s: I/O Error!\n", argv[0], argv[1]);
			} else if (res == EINVPATH) {
				kputsf ("%s: %s: Invalid PATH\n", argv[0], argv[1]);
			}
			continue;
		} else if (streq (argv[0], "help")) {
			puts ("Built-in commands:\n");
			puts ("  cd     - change current directory\n");
			puts ("  help   - show this help\n");
			puts ("  exit   - exit shell\n");
		} else {
			int result = run_cmd (argc, argv);
			if (result == EIO) {
				kputsf ("%s: Failed to load: I/O Error!\n", argv[0]);
				continue;
			} else if (result == ENOTFOUND) {
				kputsf ("%s: No such file or directory!\n", argv[0]);
				continue;
			} else if (result == EPERMDEN) {
				kputsf ("%s: Permission denied!\n", argv[0]);
				continue;
			} else if (result == ETOOLARGE) {
				kputsf ("%s: File is too large!\n", argv[0]);
				continue;
			} else if (result == ENOMEM) {
				kputsf ("%s: Failed to load: No have memory!\n", argv[0]);
				continue;
			} else if (result == ENOTFILE) {
				kputsf ("%s: Not file!\n", argv[0]);
				continue;
			} else if (result == EINVPATH) {
				kputsf ("%s: Invalid PATH!\n", argv[0]);
				continue;
			} else {
				if (result)
					kputsf ("%s: Unknown Error! (CODE %d)\n", argv[0], result);
				continue;
			}
		}
	}
}
