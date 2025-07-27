/**
 * kernel.c
 * Created by Matheus Leme Da Silva
 */
#include "types.h"
#include "console.h"

void kmain()
{
	init_console();
	printf("Hello World\n");
	for(;;);
}
