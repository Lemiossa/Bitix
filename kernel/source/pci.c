/************************************
 * pci.c                            *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#include <stdint.h>
#include <stddef.h>
#include <terminal.h>
#include <asm.h>
#include <pci.h>

pci_device_t pci_devices[MAX_PCI_DEVICES];
uint32_t pci_device_count = 0;

#define PCI_CONFIG_ADDR 0xCF8
#define PCI_CONFIG_DATA 0xCFC

/* Lê um disp do PCI */
uint32_t pci_read(uint8_t bus, uint8_t dev, uint8_t func, uint8_t reg)
{
	outl(PCI_CONFIG_ADDR,
			(1 << 31) |
			(bus << 16) |
			((dev & 31) << 11) |
			((func & 7) << 8) |
			((reg & 63) << 2)
			);
	return inl(PCI_CONFIG_DATA);
}

/* Procura um dispositivo no PCI */
pci_device_t *pci_find(uint8_t class, uint8_t subclass)
{
	for (uint32_t i = 0; i < pci_device_count; i++)
		if (pci_devices[i].class == class &&
			pci_devices[i].subclass == subclass)
			return &pci_devices[i];
	return NULL;
}

/* Enumera todos os PCI */
void pci_enumerate(void)
{
	for (uint16_t bus = 0; bus < 256; bus++) {
		for (uint8_t dev = 0; dev < 32; dev++) {
			for (uint8_t func = 0; func < 8; func++) {
				uint32_t id = pci_read(bus, dev, func, 0);
				if ((id & 0xFFFF) == 0xFFFF) {
					if (func == 0)
						break;
					continue;
				}

				uint16_t vendor_id = id & 0xFFFF;
				uint16_t device_id = (id >> 16) & 0xFFFF;

				pci_device_t device;
				uint32_t reg2 = pci_read(bus, dev, func, 2);
				uint8_t class = (reg2 >> 24) & 0xFF;
				uint8_t subclass  = (reg2 >> 16) & 0xFF;
				device.bus = bus;
				device.dev = dev;
				device.func = func;
				device.class = class;
				device.subclass = subclass;
				device.vendor_id = vendor_id;
				device.device_id = device_id;

				uint32_t reg3 = pci_read(bus, dev, func, 3);
				uint8_t header_type = (reg3 >> 16) & 0x7F; /* Ignora o bit de multifunção */
				if (header_type == 0x00) {
					for (int i = 0; i < 6; i++)
						device.bars[i] = pci_read(bus, dev, func, 4 + i);
				} else {
					for (int i = 0; i < 2; i++)
						device.bars[i] = pci_read(bus, dev, func, 4 + i);
				}

				pci_devices[pci_device_count++] = device;

				printf("PCI %hu:%hhu:%hhu vendor=0x%04X class=%hhu sub=%hhu\r\n",
					bus, dev, func,
					vendor_id, class, subclass);
			}
		}
	}
}

