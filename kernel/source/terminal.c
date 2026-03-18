/************************************
 * terminal.c                       *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#include <asm.h>
#include <boot.h>
#include <graphics.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <terminal.h>
#include <vga.h>
#include <panic.h>
#include <stdbool.h>
#include <heap.h>
#include <sched.h>
#include <font8x8.h>

static int char_height = 8;
static int width = 80, height = 25;
static int top_corner_x = 0, top_corner_y = 0;
static int bottom_corner_x = 80, bottom_corner_y = 25;

static int cursor_x = 0, cursor_y = 0;
static uint8_t current_fg_color = 7;
static uint8_t current_bg_color = 0;
static bool initialized = false;

static uint8_t *font = NULL;

typedef struct char_cell
{
	char ch;
	uint8_t fg, bg;
} char_cell_t;

static char_cell_t *buffer = NULL;

static uint32_t terminal_palette[16] = {
	RGB(0, 0, 0),		/* Preto */
	RGB(170, 0, 0),		/* Vermelho */
	RGB(0, 170, 0),		/* Verde */
	RGB(170, 85, 0),	/* Amarelo */
	RGB(0, 0, 170),		/* Azul */
	RGB(170, 0, 170),	/* Roxo */
	RGB(0, 170, 170),	/* Ciano */
	RGB(170, 170, 170), /* Cinza claro */
	RGB(85, 85, 85),	/* Cinza escuro */
	RGB(255, 85, 85),	/* Vermelho claro */
	RGB(85, 255, 85),	/* Verde claro */
	RGB(255, 255, 85),	/* Amarelo claro */
	RGB(85, 85, 255),	/* Azul claro */
	RGB(255, 85, 255),	/* Roxo claro */
	RGB(85, 255, 255),	/* Ciano claro */
	RGB(255, 255, 255)	/* Branco */
};

/* Desenha um caractere da fonte VGA */
static void draw_char(int x, int y, char c, uint32_t fg, uint32_t bg)
{
	if (boot_info.graphics.mode == 0)
	{
		uint8_t attr = ((terminal_palette[bg] & 0x0F) << 4) |
					   (terminal_palette[fg] & 0x0F);
		vga_put_char(x, y, c, attr);
		return;
	}

	if (!font)
		return;

	int height = char_height;

	int dx = x * 8;
	int dy = y * height;

	int pos = c * height;

	uint8_t *glyph = &font[pos];

	for (int cy = 0; cy < height; cy++)
	{
		uint8_t line = glyph[cy];

		for (int cx = 0; cx < 8; cx++)
		{
			if (line & (0x80 >> cx))
			{
				graphics_put_pixel(cx + dx, cy + dy, fg);
			}
			else
			{
				graphics_put_pixel(cx + dx, cy + dy, bg);
			}
		}
	}
}

/* Desenha um caractere numa posição específica */
static void put_char_at(int x, int y, char_cell_t cell)
{
	if (x < 0 || y < 0 || x >= width || y >= height || !buffer)
		return;

	uint16_t pos = y * width + x;
	buffer[pos] = cell;
}

/* pega um caractere numa posição específica do terminal */
static char_cell_t *get_char_at(int x, int y)
{
	if (x < 0 || y < 0 || x >= width || y >= height || !buffer)
		return NULL;

	uint16_t pos = y * width + x;
	return &buffer[pos];
}

/* Desenha o cursor */
static void draw_cursor(int x, int y)
{
	char_cell_t *cell = get_char_at(x, y);
	draw_char(x, y, cell->ch, terminal_palette[cell->bg],
			  terminal_palette[cell->fg]);
}

/* "Limpa" o cursor */
static void erase_cursor(int x, int y)
{
	char_cell_t *cell = get_char_at(x, y);
	draw_char(x, y, cell->ch, terminal_palette[cell->fg],
			  terminal_palette[cell->bg]);
}

/* Redesenha todo o terminal */
static void redraw(void)
{
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			char_cell_t *cell = get_char_at(x, y);
			if (boot_info.graphics.mode == 0)
			{
				uint8_t attributes = (uint8_t)terminal_palette[cell->fg] |
									 ((uint8_t)terminal_palette[cell->bg] << 4);
				vga_put_char(x, y, cell->ch, attributes);
			}
			else
			{
				draw_char(x, y, cell->ch, terminal_palette[cell->fg],
						  terminal_palette[cell->bg]);
			}
		}
	}
}

/* Faz scroll de 1 linha */
/* Apenas altera o buffer */
static void scroll(void)
{
	for (int y = top_corner_y - 1; y < bottom_corner_y; y++)
	{
		for (int x = top_corner_x; x < bottom_corner_x; x++)
		{
			char_cell_t *current_cell = get_char_at(x, y);
			char_cell_t *next_cell = get_char_at(x, y + 1);
			if (!current_cell || !next_cell)
				continue;

			*current_cell = *next_cell;
		}
	}

	for (int x = top_corner_x; x < bottom_corner_x; x++)
	{
		char_cell_t *cell = get_char_at(x, bottom_corner_y - 1);
		if (!cell)
			continue;

		cell->ch = ' ';
		cell->bg = current_bg_color;
		cell->fg = current_fg_color;
	}
}

/* Inicializa sistema do terminal */
void terminal_init(void)
{
	font = (uint8_t *)bitixfont;

	if (boot_info.graphics.mode == 1)
	{
		width = boot_info.graphics.width / 8;
		height = boot_info.graphics.height / char_height;
		top_corner_x = 0;
		top_corner_y = 0;
		bottom_corner_x = width;
		bottom_corner_y = height;
	}
	else
	{
		vga_disable_cursor(); /* Desabilitar o cursor porque desenharemos nosso
								 proprio */
	}

	buffer = alloc(width * height * sizeof(char_cell_t));
	if (!buffer)
		panic("Terminal: Falha ao alocar memoria para o buffer\r\n");

	if (boot_info.graphics.bpp <= 8)
	{
		/* Mudar a paleta para VGA */
		terminal_palette[0] = 0;   /* Preto */
		terminal_palette[1] = 4;   /* Vermelho */
		terminal_palette[2] = 2;   /* Verde */
		terminal_palette[3] = 6;   /* Marrom */
		terminal_palette[4] = 1;   /* Azul */
		terminal_palette[5] = 5;   /* Roxo */
		terminal_palette[6] = 3;   /* Ciano */
		terminal_palette[7] = 7;   /* Cinza */
		terminal_palette[8] = 8;   /* Cinza escuro */
		terminal_palette[9] = 12;  /* Vermelho brilhante */
		terminal_palette[10] = 10; /* Verde brilhante */
		terminal_palette[11] = 14; /* Amarelo */
		terminal_palette[12] = 9;  /* Azul brilhante */
		terminal_palette[13] = 13; /* Roxo brilhante */
		terminal_palette[14] = 11; /* Ciano brilhante */
		terminal_palette[15] = 15; /* Branco*/
	}

	current_bg_color = TERMINAL_DEFAULT_BG_COLOR;
	current_fg_color = TERMINAL_DEFAULT_FG_COLOR;
	initialized = true;
}

/* Muda a posição do cursor */
void terminal_set_cursor(int x, int y)
{
	if (boot_info.graphics.mode == 0)
	{
		vga_set_cursor(x, y);
		return;
	}

	cursor_x = x;
	cursor_y = y;
}

/* Limpa toda a tela */
void terminal_clear(uint8_t fg, uint8_t bg)
{
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			char_cell_t *cell = get_char_at(x, y);
			cell->ch = ' ';
			cell->fg = fg & 0x0F;
			cell->bg = bg & 0x0F;
		}
	}
	redraw();
	terminal_set_cursor(top_corner_x, top_corner_y);
	current_fg_color = fg;
	current_bg_color = bg;
}

/* Imprime um caractere na tela */
void terminal_putchar(char c)
{
	if (!initialized)
		return;

	static int state = 0; /* 0 = normal, 1 = escape, 2 = csi */
	static int csi_param_table[TERMINAL_MAX_CSI_PARAMS];
	static int csi_params = 0; /* Quantidade de parametros */

	erase_cursor(cursor_x, cursor_y);
	if (c == '\n')
	{
		cursor_y++;
	}
	else if (c == '\r')
	{
		cursor_x = top_corner_x;
	}
	else if (c == '\t')
	{
		cursor_x += 4;
	}
	else if (c == '\b')
	{
		if (cursor_x > 0)
			cursor_x--;
	}
	else if (c == '\033')
	{ /* Ativar modo escape */
		state = 1;
	}
	else if (c == '[' && state == 1)
	{ /* Ativar o modo CSI */
		state = 2;
	}
	else if (c >= '0' && c <= '9' && state == 2)
	{ /* Parametros */
		csi_param_table[csi_params] =
			csi_param_table[csi_params] * 10 + (c - '0');
	}
	else if (c == ';' && state == 2)
	{ /* Novo parametro */
		if (csi_params < TERMINAL_MAX_CSI_PARAMS)
			csi_params++;
	}
	else if (c == 'm' && state == 2)
	{ /* Sequencia SGR */
		for (int i = 0; i <= csi_params; i++)
		{
			int param = csi_param_table[i];

			if (param >= 30 && param <= 37)
			{
				current_fg_color = param - 30;
			}
			else if (param >= 40 && param <= 47)
			{
				current_bg_color = param - 40;
			}
			else if (param >= 90 && param <= 97)
			{
				current_fg_color = param - 90 + 8;
			}
			else if (param >= 100 && param <= 107)
			{
				current_fg_color = param - 100 + 8;
			}
			else if (param == 1)
			{
				if (current_fg_color < 8)
					current_fg_color += 8;
			}
			else if (param == 0)
			{
				current_bg_color = TERMINAL_DEFAULT_BG_COLOR;
				current_fg_color = TERMINAL_DEFAULT_FG_COLOR;
			}
		}
		csi_params = 0;
		memset(csi_param_table, 0, sizeof(csi_param_table));
		state = 0;
	}
	else if (c == 'P' && state == 2)
	{ /* Mudar tabela de cores */
		if (csi_params < 3)
			return;
		int idx = csi_param_table[0];
		int r = csi_param_table[1];
		int g = csi_param_table[2];
		int b = csi_param_table[3];

		if (idx > 15)
			return;

		terminal_palette[idx] = RGB(r, g, b);
		csi_params = 0;
		memset(csi_param_table, 0, sizeof(csi_param_table));
		state = 0;
	}
	else
	{
		state = 0;
		char_cell_t *cell = get_char_at(cursor_x, cursor_y);
		if (!cell)
			return;
		cell->bg = current_bg_color;
		cell->fg = current_fg_color;
		cell->ch = c;

		put_char_at(cursor_x, cursor_y, *cell);

		draw_char(cursor_x, cursor_y, cell->ch, terminal_palette[cell->fg],
			terminal_palette[cell->bg]);

		cursor_x++;
	}

	if (cursor_x >= bottom_corner_x)
	{
		cursor_y++;
		cursor_x = top_corner_x;
	}

	if (cursor_y >= bottom_corner_y)
	{
		scroll();
		redraw();
		cursor_y = bottom_corner_y - 1;
		cursor_x = top_corner_x;
	}

	terminal_set_cursor(cursor_x, cursor_y);
	draw_cursor(cursor_x, cursor_y);
}

/* Imprime uma string na tela */
void terminal_putstring(const char *s)
{
	while (*s)
	{
		terminal_putchar(*s++);
	}
}

/* Imprime uma string formatada */
int printf(const char *fmt, ...)
{
	va_list args;
	char buffer[256];
	int count = 1;
	va_start(args, fmt);
	count = vsnprintf(buffer, sizeof(buffer), fmt, args);
	terminal_putstring(buffer);
	va_end(args);
	return count;
}
