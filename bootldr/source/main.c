/************************************
 * main.c                           *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>
#include "stdio.h"
#include "disk.h"
#include "fat.h"
#include "vga.h"

/* Func principal do bootloader */
int main(void)
{
	vga_clear(0x07);
	printf("Ola mundo!\r\n");

	if (fat_init(boot_drive, 0) != 0) { /* FLOPPY */
		printf("Falha ao inicializar sistema FAT\r\n");
		goto halt;
	}

	fat_list_root();

halt:
	printf("Sistema travado. Por favor, reinicie.\r\n");
	while (1);
}

