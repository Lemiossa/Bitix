/************************************
 * graphics.c                       *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#include <asm.h>
#include <stdbool.h>
#include <stdint.h>
#include <graphics.h>
#include <string.h>
#include <terminal.h>
#include <boot.h>
#include <panic.h>
#include <pmm.h>
#include <vmm.h>
#include <debug.h>

static uint32_t bytes_per_pixel = 0;
static bool initialized = false;

/* Retorna o ponteiro no framebuffer usando x e y */
static inline uint32_t get_ptr(int x, int y)
{
	if (!initialized)
		return 0;

	static int last_y = -1;
	static uint32_t row;

	if (y != last_y)
	{
		row = boot_info.graphics.framebuffer +
			  y * boot_info.graphics.pitch;
		last_y = y;
	}

	return row + x * bytes_per_pixel;
}

/* Inicializa graficos */
/* Mapeia framebuffer */
void graphics_init(void)
{
	if (!boot_info.graphics.framebuffer)
		return;

	uint32_t size = (uint32_t)boot_info.graphics.pitch *
					(uint32_t)boot_info.graphics.height;

	bytes_per_pixel = boot_info.graphics.bpp / 8;

	if (!paging_enabled)
	{
		initialized = true;
		return;
	}

	for (uint32_t i = 0; i < size; i += PAGE_SIZE)
	{
		if (!vmm_map(boot_info.graphics.framebuffer + i,
				boot_info.graphics.framebuffer + i, PAGE_PRESENT | PAGE_WRITE))
			panic("Graficos: Falha ao mapear framebuffer de video\r\n");
	}

	initialized = true;

}

/* Desenha um PIXEL no modo grafico */
void graphics_put_pixel(int x, int y, uint32_t color)
{
	if (x >= boot_info.graphics.width || y >= boot_info.graphics.height ||
		!boot_info.graphics.mode || x < 0 || y < 0)
		return;

	/* Cache de cores */
	static uint32_t input = 0;
	static uint32_t output = 0;

	uint32_t val;
	if (color != input)
	{
		if (boot_info.graphics.bpp > 8)
		{
			uint8_t cr = (color >> 16) & 0xFF;
			uint8_t cg = (color >> 8) & 0xFF;
			uint8_t cb = (color >> 0) & 0xFF;

			uint8_t r = (cr * ((1 << boot_info.graphics.red_mask) - 1)) / 255;
			uint8_t g = (cg * ((1 << boot_info.graphics.green_mask) - 1)) / 255;
			uint8_t b = (cb * ((1 << boot_info.graphics.blue_mask) - 1)) / 255;

			val = r << boot_info.graphics.red_position;
			val |= g << boot_info.graphics.green_position;
			val |= b << boot_info.graphics.blue_position;
		}
		else
		{
			val = color;
		}
		input = color;
		output = val;
	}
	else
	{
		val = output;
	}

	uint32_t pos = get_ptr(x, y);
	if (pos == 0)
		return;

	*(uint32_t *)pos = val;
}

/* Retorna um pixel no modo grafico */
uint32_t graphics_get_pixel(int x, int y)
{
	if (x >= boot_info.graphics.width || y >= boot_info.graphics.height ||
		!boot_info.graphics.mode || x < 0 || y < 0 || !initialized)
		return 0;

	uint32_t pos = get_ptr(x, y);
	if (pos == 0)
		return 0;

	uint32_t val = *(uint32_t *)pos;
	uint32_t ret = 0;
	if (boot_info.graphics.bpp > 8)
	{
		uint8_t r = 0, g = 0, b = 0;

		r = (val >> boot_info.graphics.red_position) &
			((1 << boot_info.graphics.red_mask) - 1);
		g = (val >> boot_info.graphics.green_position) &
			((1 << boot_info.graphics.green_mask) - 1);
		b = (val >> boot_info.graphics.blue_position) &
			((1 << boot_info.graphics.blue_mask) - 1);

		ret = r | (g << 8) | (b << 16);
	}
	else
	{
		ret = val;
	}

	return ret;
}

/* Desenha uma reta no modo grafico */
/* Metodo Bresenham simplificado */
void graphics_put_line(int x0, int y0, int x1, int y1, uint32_t color)
{
	if (!boot_info.graphics.mode || !initialized)
		return;

	int dx = x1 - x0;
	int dy = y1 - y0;
	int d = (2 * dy) - dx;
	int y = y0;

	for (int x = x0; x <= x1; x++)
	{
		graphics_put_pixel(x, y, color);
		if (d > 0)
		{
			y++;
			d += 2 * (dy - dx);
		}
		else
		{
			d += 2 * dy;
		}
	}
}

/* Faz um scroll no framebuffer gráfico */
void graphics_scroll(int topx, int topy, int bottomy, int pixels)
{
	if (!boot_info.graphics.mode || !initialized)
		return;

	uint32_t dest_pos = get_ptr(topx, topy);
	uint32_t src_pos = get_ptr(topx, topy + pixels);
	uint32_t length = (bottomy - topy - pixels) * boot_info.graphics.pitch;
	memcpy((void *)dest_pos, (void *)src_pos, length);
}
