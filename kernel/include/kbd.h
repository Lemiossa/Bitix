/************************************
 * kbd.h                            *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#ifndef KBD_H
#define KBD_H
#include <stdint.h>

typedef struct kbd_event {
	uint8_t sc;
	uint8_t mod;
	bool released;
} kbd_event_t;

bool kbd_read_event(kbd_event_t *out);
bool kbd_write_event(kbd_event_t event);
void kbd_init(void);

#endif /* KBD_H */
