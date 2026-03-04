/************************************
 * config.h                         *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#ifndef CONFIG_H
#define CONFIG_H
#include <stddef.h>

#define KERNEL_FILE "/system/boot/kernel.sys"
#define KERNEL_ADDR 0x100000

/* Lista de modos */
/* Ele vai tentar entrar em um, se falhar, tenta o proximo, por assim vai  */
char *video_modes[] = {
	/*NULL,*/ /* Descomente isso se quiser mudar pro modo texto */
	/* 800x600 */
	"800x600x32",
	"800x600x24",
	"800x600x16",
	"800x600x8",

	/* 640x480 */
	"640x480x32",
	"640x480x24",
	"640x480x16",
	"640x480x8"
};

#define FONT 0x03

#endif /* CONFIG_H */
