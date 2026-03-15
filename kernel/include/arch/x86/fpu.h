/************************************
 * fpu.h                            *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#ifndef FPU_H
#define FPU_H
#include <stdbool.h>

void fpu_init(void);
void fpu_create_new_context(void *out);

#endif /* FPU_H */
