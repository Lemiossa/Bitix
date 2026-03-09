/************************************
 * pic.h                            *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#ifndef PIC_H
#define PIC_H
#include <stdint.h>

void pic_remap(uint8_t offset1, uint8_t offset2);
void pic_eoi(uint8_t irq);
void pic_mask_irq(uint8_t irq);
void pic_unmask_irq(uint8_t irq);

#endif /* PIC_H */
