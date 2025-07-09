/**
 * echo.c
 * Created by Matheus Leme Da Silva
 */
#include <userlib.h>

int
main (int argc, char *argv[])
{
	if (argc > 1) {
		int i;
		for (i = 0; i < argc; i++) {
			if (i == 0)
				continue;
			printf ("%s ", argv[i]);
		}
		printf ("\n");
	} else
		printf ("Usage: %s strings...\n", argv[0]);
}
