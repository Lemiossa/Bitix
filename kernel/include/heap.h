/************************************
 * heap.h                           *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#ifndef HEAP_H
#define HEAP_H
#include <stddef.h>

void heap_init(void);
void *alloc(size_t n);
void free(void *p);

#endif /* HEAP_H */
