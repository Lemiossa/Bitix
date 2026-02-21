/************************************
 * e820.h                           *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#ifndef E820_H
#define E820_H
#include <stdint.h>

typedef struct e820_entry {
	uint64_t base;
	uint64_t length;
	uint32_t type;
	uint32_t attr;
} __attribute__((packed)) e820_entry_t;

int E820_get_table(e820_entry_t *out, int max);

#endif /* E820_H */
