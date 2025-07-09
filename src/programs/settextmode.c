/**
 * settextmode.c
 * Created by Matheus Leme Da Silva
 */
#include <userlib.h>

int
main (int argc, char *argv[])
{
	int mode;
	if (argc < 2) {
		printf ("Usage: %s <mode>\n", argv[0]);
		printf ("Modes:\n");
		printf ("\t40x25\n");
		printf ("\t80x25\n");
		printf ("\t80x50\n");
		return 1;
	}
	if (streq (argv[1], "40x25"))
		mode = 1;
	else if (streq (argv[1], "80x25"))
		mode = 2;
	else if (streq (argv[1], "80x50"))
		mode = 3;
	else {
		printf ("Unknown mode: %s\n", argv[1]);
		return 1;
	}
	setvid (mode);
	clear_screen (0x07);
	printf ("Switched to mode: %s\n", argv[1]);
	return 0;
}
