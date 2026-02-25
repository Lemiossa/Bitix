/************************************
 * terminal.c                       *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <boot.h>
#include <string.h>
#include <vga.h>
#include <io.h>
#include <graphics.h>
#include <stddef.h>
#include <timer.h>
#include <terminal.h>

#define CELLS 180 * 80

static int char_height = 8;
static int width = 80, height = 25;
static int top_corner_x = 0, top_corner_y = 0;
static int bottom_corner_x = 80, bottom_corner_y = 25;

static int cursor_x = 0, cursor_y = 0;
static uint8_t current_fg_color = 7;
static uint8_t current_bg_color = 0;

static int cursor_visible = 1;
static int last_x = 0;
static int last_y = 0;

typedef struct char_cell {
	char ch;
	uint8_t fg, bg;
} char_cell_t;

static char_cell_t buffer[CELLS];

static uint32_t terminal_pallete[16] = {
	0x000000, /* Preto */
	0xAA0000, /* Vermelho */
	0x00AA00, /* Verde */
	0xAA5500, /* Marrom  */
	0x0000AA, /* Azul */
	0xAA00AA, /* Roxo */
	0x00AAAA, /* Ciano */
	0xAAAAAA, /* Cinza */
	0x555555, /* Cinza escuro */
	0xFF5555, /* Vermelho brilhante */
	0x55FF55, /* Verde brilhante */
	0xFFFF55, /* Amarelo */
	0x5555FF, /* Azul brilhante */
	0xFF55FF, /* Roxo brilhante */
	0x55FFFF, /* Ciano brilhante */
	0xFFFFFF  /* Branco */
};

/* Desenha um caractere da fonte VGA */
static void draw_char(int x, int y, char c, uint32_t fg, uint32_t bg)
{
	if (!boot_info.vga_font || boot_info.graphics.mode == 0)
		return;

	int height = char_height;

	int dx = x * 8;
	int dy = y * height;

	int pos = c * height;

	uint8_t *glyph = &boot_info.vga_font[pos];

	for (int cy = 0; cy < height; cy++) {
		uint8_t line = glyph[cy];

		for (int cx = 0; cx < 8; cx++) {
			if (line & (0x80 >> cx)) {
				put_pixel(cx + dx, cy + dy, fg);
			} else {
				put_pixel(cx + dx, cy + dy, bg);
			}
		}
	}
}

/* Desenha um caractere numa posição específica */
static void put_char_at(int x, int y, char_cell_t cell)
{
	if (x < 0 || y < 0 || x >= width || y >= height)
		return;

	uint8_t bg = cell.bg;
	uint8_t fg = cell.fg;
	uint8_t c = cell.ch;

	if (boot_info.graphics.mode == 0) {
		uint8_t attr = ((terminal_pallete[bg] & 0x0F) << 4) | (terminal_pallete[fg] & 0x0F);
		vga_put_char(x, y, c, attr);
		return;
	}

	draw_char(x, y, c, terminal_pallete[fg], terminal_pallete[bg]);
}

/* pega um caractere numa posição específica do terminal */
static char_cell_t *get_char_at(int x, int y)
{
	if (x < 0 || y < 0 || x >= width || y >= height)
		return NULL;

	uint16_t pos = y * width + x;
	return &buffer[pos];
}

/* Desenha o cursor */
static void draw_cursor(int x, int y)
{
	char_cell_t *cell = get_char_at(x, y);
	if (!cell) return;

	uint8_t fg = cell->fg;
	uint8_t bg = cell->bg;

	draw_char(x, y, cell->ch,
		terminal_pallete[bg],
		terminal_pallete[fg]);
}

/* "Limpa" o cursor */
static void erase_cursor(int x, int y)
{
	char_cell_t *cell = get_char_at(x, y);
		if (!cell) return;

	put_char_at(x, y, *cell);
}

/* Redesenha todo o terminal */
static void redraw(void)
{
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			char_cell_t *cell = get_char_at(x, y);
			put_char_at(x, y, *cell);
		}
	}
	draw_cursor(cursor_x, cursor_y);
}

/* Redesenha a linha atual */
static void redraw_line(void) {
	for (int x = top_corner_x; x < bottom_corner_x; x++) {
		char_cell_t *cell = get_char_at(x, cursor_y);
		put_char_at(x, cursor_y, *cell);
	}
}

/* Faz scroll de 1 linha */
/* Apenas altera o buffer */
static void scroll(void)
{
	for (int y = top_corner_y + 1; y < bottom_corner_y; y++) {
		for (int x = top_corner_x; x < bottom_corner_x; x++) {
			char_cell_t *cell = get_char_at(x, y);
			char_cell_t *cell0 = get_char_at(x, y - 1);
			*cell0 = *cell;
		}
	}

	for (int x = top_corner_x; x < bottom_corner_x; x++) {
		char_cell_t *cell = get_char_at(x, bottom_corner_y - 1);
		cell->ch = ' ';
	}
}

/* Inicializa sistema do terminal */
void terminal_init(void)
{
	if (boot_info.graphics.mode == 1) {

		width = boot_info.graphics.width / 8;
		height = boot_info.graphics.height / char_height;
		top_corner_x = 0;
		top_corner_y = 0;
		bottom_corner_x = width;
		bottom_corner_y = height;
	}

	if (boot_info.graphics.bpp <= 8) {
		/* Mudar a paleta para VGA */
		terminal_pallete[0] = 0; /* Preto */
		terminal_pallete[1] = 4; /* Vermelho */
		terminal_pallete[2] = 2; /* Verde */
		terminal_pallete[3] = 6; /* Marrom */
		terminal_pallete[4] = 1; /* Azul */
		terminal_pallete[5] = 5; /* Roxo */
		terminal_pallete[6] = 3; /* Ciano */
		terminal_pallete[7] = 7; /* Cinza */
		terminal_pallete[8] = 8; /* Cinza escuro */
		terminal_pallete[9] = 12; /* Vermelho brilhante */
		terminal_pallete[10] = 10; /* Verde brilhante */
		terminal_pallete[11] = 14; /* Amarelo */
		terminal_pallete[12] = 9; /* Azul brilhante */
		terminal_pallete[13] = 13; /* Roxo brilhante */
		terminal_pallete[14] = 11; /* Ciano brilhante */
		terminal_pallete[15] = 15; /* Branco*/
	}

	current_bg_color = 0;
	current_fg_color = 7;
}

/* Muda a posição do cursor */
void terminal_set_cursor(int x, int y)
{
	if (boot_info.graphics.mode == 0) {
		vga_set_cursor(x, y);
		return;
	}

	cursor_x = x;
	cursor_y = y;
}

/* Limpa toda a tela */
void terminal_clear(uint8_t fg, uint8_t bg)
{
	erase_cursor(cursor_x, cursor_y);
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			char_cell_t *cell = get_char_at(x, y);
			cell->ch = ' ';
			cell->fg = fg & 0x0F;
			cell->bg = bg & 0x0F;
		}
	}
	redraw();
	terminal_set_cursor(top_corner_x, top_corner_y);
}

#define PORT 0x3f8          // COM1

static int init_serial() {
   outb(PORT + 1, 0x00);    // Disable all interrupts
   outb(PORT + 3, 0x80);    // Enable DLAB (set baud rate divisor)
   outb(PORT + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
   outb(PORT + 1, 0x00);    //                  (hi byte)
   outb(PORT + 3, 0x03);    // 8 bits, no parity, one stop bit
   outb(PORT + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
   outb(PORT + 4, 0x0B);    // IRQs enabled, RTS/DSR set
   outb(PORT + 4, 0x1E);    // Set in loopback mode, test the serial chip
   outb(PORT + 0, 0xAE);    // Test serial chip (send byte 0xAE and check if serial returns same byte)

   // Check if serial is faulty (i.e: not same byte as sent)
   if(inb(PORT + 0) != 0xAE) {
      return 1;
   }

   // If serial is not faulty set it in normal operation mode
   // (not-loopback with IRQs enabled and OUT#1 and OUT#2 bits enabled)
   outb(PORT + 4, 0x0F);
   return 0;
}


int is_transmit_empty() {
   return inb(PORT + 5) & 0x20;
}

void write_serial(char a) {
   while (is_transmit_empty() == 0);

   outb(PORT,a);
}

int initialized = 0;

/* Imprime um caractere na tela */
void terminal_putchar(char c)
{
	if (initialized == 0) {init_serial(); initialized = 1;}
	write_serial(c);
	if (c == '\n') {
		cursor_y++;
	} else if (c == '\r') {
		cursor_x = top_corner_x;
	} else if (c == '\t') {
		cursor_x += 4;
	} else if (c == '\b') {
		if (cursor_x > 0)
			cursor_x--;
	} else {
		char_cell_t *cell = get_char_at(cursor_x, cursor_y);
		cell->bg = current_bg_color;
		cell->fg = current_fg_color;
		cell->ch = c;
		put_char_at(cursor_x++, cursor_y, *cell);
	}

	if (cursor_x >= bottom_corner_x) {
		cursor_y++;
		cursor_x = top_corner_x;
	}

	if (cursor_y >= bottom_corner_y) {
		scroll();
		redraw();
		cursor_y = bottom_corner_y - 1;
		cursor_x = top_corner_x;
	}

	terminal_set_cursor(cursor_x, cursor_y);
}


/* Tick de um terminal pra piscar o cursor */
static uint32_t ticks = 0;
void terminal_tick(void)
{
	if (boot_info.graphics.mode != 0 &&
			ticks >= timer_ms_to_tick(TERMINAL_CURSOR_BLINK_MS)) {
		cursor_visible = !cursor_visible;
		ticks = 0;
		if (cursor_visible)
			draw_cursor(cursor_x, cursor_y);
		else
			erase_cursor(cursor_x, cursor_y);
	}

	ticks++;
}

/* Imprime uma string na tela */
void terminal_putstring(const char *s)
{
	while (*s) {
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
	count = vsprintf(buffer, fmt, args);
	terminal_putstring(buffer);
	va_end(args);
	return count;
}

