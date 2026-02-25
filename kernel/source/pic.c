/************************************
 * pic.c                            *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#include <stdint.h>
#include <io.h>

#define MASTER 0x20
#define SLAVE 0xA0
#define MASTER_CMD MASTER
#define SLAVE_CMD SLAVE
#define MASTER_DATA ((MASTER)+1)
#define SLAVE_DATA ((SLAVE)+1)

#define ICW1_INIT 0x10
#define ICW1_IC4  0x01
#define ICW4_8086_MODE 0x01

#define CASCADE_IRQ 0x02

/* Remapeia PICs */
void pic_remap(uint8_t offset1, uint8_t offset2)
{
	/* Master */
	outb(MASTER_CMD, ICW1_INIT | ICW1_IC4);
	io_wait();
	outb(MASTER_DATA, offset1);
	io_wait();
	outb(MASTER_DATA, 1 << CASCADE_IRQ); /* No master, passa o ICW3 em bitmask */
	io_wait();
	outb(MASTER_DATA, ICW4_8086_MODE);
	io_wait();

	/* Slave */
	outb(SLAVE_CMD, ICW1_INIT | ICW1_IC4);
	io_wait();
	outb(SLAVE_DATA, offset2);
	io_wait();
	outb(SLAVE_DATA, CASCADE_IRQ); /* No slave, passa o ICW3 em número */
	io_wait();
	outb(SLAVE_DATA, ICW4_8086_MODE);
	io_wait();

	/* Mascarar todos os IRQ */
	outb(MASTER_DATA, 0xFF);
	outb(SLAVE_DATA, 0xFF);
}

/* Envia EOI pro PIC */
void pic_eoi(uint8_t irq)
{
	if (irq >= 8)
		outb(SLAVE_CMD, 0x20);
	outb(MASTER_CMD, 0x20);
}

/* Mascara uma IRQ no PIC */
void pic_mask_irq(uint8_t irq)
{
	if (irq < 8) {
		outb(MASTER_DATA, inb(MASTER_DATA) | (1 << irq));
	} else {
		outb(SLAVE_DATA, inb(SLAVE_DATA) | (1 << (irq - 8)));
	}
}

/* Desmascara uma IRQ no PIC */
void pic_unmask_irq(uint8_t irq)
{
	if (irq < 8) {
		outb(MASTER_DATA, inb(MASTER_DATA) & ~(1 << irq));
	} else {
		outb(SLAVE_DATA, inb(SLAVE_DATA) & ~(1 << (irq - 8)));
	}
}
