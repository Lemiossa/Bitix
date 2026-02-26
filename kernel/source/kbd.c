/************************************
 * kbd.c                            *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#include <stdint.h>
#include <stdbool.h>

#include <io.h>
#include <pic.h>

#include <kbd.h>

#define KBD_MAX_EVENTS 256
#define KBD_MASK (KBD_MAX_EVENTS - 1)

#define KBD_MOD_LCTRL   (1 << 0)
#define KBD_MOD_RCTRL   (1 << 1)
#define KBD_MOD_ALT     (1 << 2)
#define KBD_MOD_SHIFT   (1 << 3)
#define KBD_MOD_CAPS    (1 << 4)

#define KBD_LED_SCROLL  (1 << 0)
#define KBD_LED_NUMBER  (1 << 1)
#define KBD_LED_CAPS    (1 << 2)

static int count = 0;
static int head = 0;
static int tail = 0;
static kbd_event_t buffer[KBD_MAX_EVENTS];

static uint8_t leds = KBD_LED_NUMBER;
static uint8_t mod = 0;

/* Muda os leds do teclado */ 
void kbd_set_leds(uint8_t leds)
{
	outb(0x64, 0xED);
	outb(0x60, leds);
}

/* Retorna true se o ring buffer do teclado está cheio */ 
bool kbd_buffer_is_full(void)
{
	return count >= KBD_MAX_EVENTS;
}

/* Retorna true se o ring buffer do teclado está vazio */ 
bool kbd_buffer_is_empty(void)
{
	return count <= 0;
}

/* Lê um evento de teclado */
/* Retorna true se não houver erro */
bool kbd_read_event(kbd_event_t *out)
{
	if (kbd_buffer_is_empty())
		return false;

	*out = buffer[tail];
	tail = (tail + 1) & KBD_MASK;
	count--;

	return true;
}

/* Escreve um evento de teclado */
/* Retorna true se não houver erro */
bool kbd_write_event(kbd_event_t event)
{
	if (kbd_buffer_is_full())
		return false;

	buffer[head] = event;
	head = (head + 1) % KBD_MASK;
	count++;

	return true;
}

/* Handler de teclado */
void kbd_handler(void)
{
	uint8_t sc = inb(0x60);
	bool released = false;

	if (sc & 0x80) {
		sc &= ~0x80;
		released = true;
	}

	if (sc == 0x1D) { /* LCTRL */
		mod = !released ? mod | KBD_MOD_LCTRL : mod & ~KBD_MOD_LCTRL;
		return;
	} else if (sc == 0x2A) { /* SHIFT */
		mod = !released ? mod | KBD_MOD_SHIFT : mod & ~KBD_MOD_SHIFT;
		return;
	} else if (sc == 0x38) { /* ALT */
		mod = !released ? mod | KBD_MOD_ALT : mod & ~KBD_MOD_ALT;
		return;
	} else if (sc == 0x3A) { /* CAPS */
		mod = !released ? mod | KBD_MOD_CAPS : mod & ~KBD_MOD_CAPS;
		leds = !released ? leds | KBD_LED_CAPS : leds & ~KBD_LED_CAPS;
		kbd_set_leds(leds);
		return;
	}
	
	kbd_event_t ev;
	ev.sc = sc;
	ev.mod = mod;
	ev.released = released;
	kbd_write_event(ev);

	pic_eoi(1);
}

/* Inicializa teclado  */
void kbd_init(void)
{
	/* Ativar os LEDS iniciais */
	kbd_set_leds(leds);
	pic_unmask_irq(1);
}