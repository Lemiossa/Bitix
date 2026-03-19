/************************************
 * vesa.c                           *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#include "vesa.h"
#include "real_mode.h"
#include "util.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Retorna o valor absoluto de um inteiro */
static inline int iabs(int x)
{
	return (x < 0) ? -x : x;
}

/* Pega a estrutura de informações de um modo VESA */
/* Retorna um número diferente de zero caso haja erro */
int vesa_get_mode_info(uint16_t mode, vbe_mode_info_t *mode_info)
{
	Regs r = {0};
	r.w.ax = 0x4F01;
	r.w.cx = mode;
	r.w.es = MK_SEG(mode_info);
	r.w.di = MK_OFF(mode_info);
	intx(0x10, &r);
	if (r.w.ax != 0x004F)
		return 1;

	return 0;
}

/* Seta um modo VESA */
/* Retorna um número diferente de zero caso haja erro */
int vesa_set_mode(uint16_t mode)
{
	Regs r = {0};
	r.w.ax = 0x4F02;
	r.w.bx = mode | 0x4000;
	intx(0x10, &r);

	if (r.w.ax != 0x004F)
		return 1;

	return 0;
}

/* Procura um modo proximo */
uint16_t vesa_find_mode(const char *mode)
{
	Regs r = {0};

	vbe_info_block_t info_block = {0};
	int expected_width = 640;
	int expected_height = 480;
	int expected_bpp = 8;

	sscanf(mode, "%dx%dx%d", &expected_width, &expected_height, &expected_bpp);

	strncpy(info_block.signature, "VESA", 4);

	r.w.ax = 0x4F00;
	r.w.es = MK_SEG(&info_block);
	r.w.di = MK_OFF(&info_block);
	intx(0x10, &r);
	if (r.w.ax != 0x004F)
		return 0x13;

	uint16_t *modes = (uint16_t *)MK_PTR(info_block.video_mode_seg,
										 info_block.video_mode_off);

	uint16_t best_mode = 0x13;
	int best_score = 999999;

	for (int i = 0; modes[i] != 0xFFFF; i++)
	{
		uint16_t mode = modes[i];
		if (mode == 0)
			continue;

		vbe_mode_info_t mode_info = {0};
		if (vesa_get_mode_info(mode, &mode_info) != 0)
			continue;

		if (!(mode_info.attributes & 0x80))
			continue;

		// opcional: só modos gráficos
		if (!(mode_info.attributes & 0x10))
			continue;

		int score =
			iabs((int)mode_info.width - expected_width) +
			iabs((int)mode_info.height - expected_height) +
			iabs((int)mode_info.bpp - expected_bpp) * 10;

		if (score < best_score)
		{
			best_score = score;
			best_mode = mode;
		}
	}

	return best_mode;
}
