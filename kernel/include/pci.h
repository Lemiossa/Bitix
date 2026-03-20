/************************************
 * pci.h                            *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#ifndef PCI_H
#define PCI_H
#include <stdint.h>

typedef struct pci_device
{
	uint8_t bus;
	uint8_t dev;
	uint8_t func;
	uint16_t vendor_id;
	uint16_t device_id;
	uint8_t class;
	uint8_t subclass;
	uint8_t prog_if;
	uint32_t bars[6];
} pci_device_t;

#define MAX_PCI_DEVICES 256

uint32_t pci_read(uint8_t bus, uint8_t dev, uint8_t func, uint8_t reg);
pci_device_t *pci_find(uint8_t class, uint8_t subclass);
void pci_enumerate(void);
const char *pci_get_class_string(uint16_t class, uint16_t subclass);

#endif /* PCI_H */
