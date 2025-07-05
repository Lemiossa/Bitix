/**
* shell.c
* Created by Matheus Leme Da Silva
*/
#include <utils.h>

void main()
{
	uchar buffer[256];
	for(;;){
		puts("> ");
		readline(buffer, 256);
		if(strlen(buffer)>1) {
			puts(buffer);
			putc('\n');
		}
	}
}
