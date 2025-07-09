/**
 * pwd.c
 * Created by Matheus Leme Da Silva
 */
#include <userlib.h>

#define MAX_PATHL 512
#define MAX_CWD 384

int
main (int argc, char *argv[])
{
	uchar cwd[MAX_CWD];
	if (getcwd (cwd, MAX_CWD) <= 0) {
		printf ("%s: Failed to get current directory\n", argv[0]);
		return 1;
	}
	printf ("%s\n", cwd);
}
