/************************************
 * vesa.c                           *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "real_mode.h"
#include "util.h"
#include "vesa.h"

/* Seta um modo VESA */
/* Retorna um número diferente de zero caso haja erro */
int vesa_set_mode(uint16_t mode, vbe_mode_info_t *mode_info)
{
	Regs r = {0};
	r.w.ax = 0x4F01;
	r.w.cx = mode;
	r.w.es = MK_SEG(mode_info);
	r.w.di = MK_OFF(mode_info);
	intx(0x10, &r);
	if (r.w.ax != 0x004F)
		return 1;

	r.w.ax = 0x4F02;
	r.w.bx = mode | 0x4000;
	intx(0x10, &r);

	if (r.w.ax != 0x004F)
		return 1;

	return 0;
}

