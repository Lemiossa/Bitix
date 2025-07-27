/**
 * main.c
 * Created by Matheus Leme Da Silva
 */
#include "types.h"
#include "ports.h"
#include "x86.h"
#include "util.h"
#include "fs.h"
#include "disk.h"

#define KERNEL_LOAD_ADDR ((void*)0x100000)

/**
 * Load a kernel
 */
void load_kernel(const char *path)
{
	int fd=open(path);
	if(fd<0) {
		printf("Failed to open %s\r\n", path);
		return;
	}

	void *kernel_addr=KERNEL_LOAD_ADDR;
	int bytes=read(fd, kernel_addr, MiB(256));
	if(bytes<=0) {
		printf("Failed to read kernel (%d bytes)\r\n", bytes);
		close(fd);
		return;
	}
	
	close(fd);
}

/**
 * Bootloader main function
 */

void main()
{
	init_fs();

	printf("Loading kernel...");
	load_kernel("/boot/bitixz");
	printf("\r\n");
	void *kernela=KERNEL_LOAD_ADDR;

	void (*kernel)(void)=(void(*)(void))kernela;
	kernel();
	
	for(;;);
}
