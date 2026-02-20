/************************************
 * e820.h                           *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#ifndef E820_H
#define E820_H
#include <stdint.h>

typedef struct E820Entry {
	uint64_t base;
	uint64_t length;
	uint32_t type;
	uint32_t attr;
} __attribute__((packed)) E820Entry;

int E820_get_table(E820Entry *out, int max);

#endif /* E820_H */
