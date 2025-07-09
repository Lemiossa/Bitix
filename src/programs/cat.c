/**
 * cat.c
 * Created by Matheus Leme Da Silva
 */
#include <userlib.h>

#define MAX_PATHL 512
#define MAX_CWD 384

int
main (int argc, char *argv[])
{
	uint i;
	slong read;
	ushort current_seg, current_off;
	uchar fullpath[MAX_PATHL];
	uchar cwd[MAX_CWD];
	uchar *path;

	if (argc < 2) {
		printf ("Usage: %s <file>\n", argv[0]);
		return 1;
	}

	path = (uchar *)argv[1];

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

	read = bfx_readfile (fullpath, FILESEG, 0, 0, MODE_EXEC);
	if (read == EIO) {
		printf ("%s: %s: Failed to load: I/O Error!\n", argv[0], argv[1]);
		return 1;
	} else if (read == ENOTFOUND) {
		printf ("%s: %s: No such file or directory!\n", argv[0], argv[1]);
		return 1;
	} else if (read == EPERMDEN) {
		printf ("%s: %s: Permission denied!\n", argv[0], argv[1]);
		return 1;
	} else if (read == ETOOLARGE) {
		printf ("%s: %s: File is too large!\n", argv[0], argv[1]);
		return 1;
	} else if (read == ENOMEM) {
		printf ("%s: %s: Failed to load: No have memory!\n", argv[0], argv[1]);
		return 1;
	} else if (read == ENOTFILE) {
		printf ("%s: %s: Not file!\n", argv[0], argv[1]);
		return 1;
	} else if (read == EINVPATH) {
		printf ("%s: %s: Invalid PATH!\n", argv[0], argv[1]);
		return 1;
	} else {
		if (read < 0)
			printf ("%s: %s: Unknown Error! (CODE %d)\n", argv[0], argv[1], read);
	}
	for (i = 0; i < read; i++) {
		putc (lreadb (FILESEG, 0 + i));
	}
}
