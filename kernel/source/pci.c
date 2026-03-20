/************************************
 * pci.c                            *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#include <asm.h>
#include <pci.h>
#include <stddef.h>
#include <stdint.h>
#include <terminal.h>
#include <debug.h>

pci_device_t pci_devices[MAX_PCI_DEVICES];
uint32_t pci_device_count = 0;

#define PCI_CONFIG_ADDR 0xCF8
#define PCI_CONFIG_DATA 0xCFC

/* Lê um disp do PCI */
uint32_t pci_read(uint8_t bus, uint8_t dev, uint8_t func, uint8_t reg)
{
	outl(PCI_CONFIG_ADDR, (1 << 31) | (bus << 16) | ((dev & 31) << 11) |
							  ((func & 7) << 8) | ((reg & 63) << 2));
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

/* Retorna uma string para cada classe e subclasse */
const char *pci_get_class_string(uint16_t class, uint16_t subclass)
{
	switch (class)
	{
		case 0x00:
			switch (subclass)
			{
				case 0x00: return "Nao classificado (sem VGA)";
				case 0x01: return "Nao classificado (compativel com VGA)";
				default: return "Nao classificado (desconhecido)";
			}

		case 0x01:
			switch (subclass)
			{
				case 0x00: return "Controlador SCSI";
				case 0x01: return "Controlador IDE";
				case 0x02: return "Controlador de disquete";
				case 0x03: return "Controlador IPI";
				case 0x04: return "Controlador RAID";
				case 0x05: return "Controlador ATA";
				case 0x06: return "Controlador SATA";
				case 0x07: return "Controlador SAS";
				case 0x08: return "Controlador NVMe";
				case 0x80: return "Armazenamento (outro)";
				default: return "Armazenamento (desconhecido)";
			}

		case 0x02:
			switch (subclass)
			{
				case 0x00: return "Controlador Ethernet";
				case 0x01: return "Controlador Token Ring";
				case 0x02: return "Controlador FDDI";
				case 0x03: return "Controlador ATM";
				case 0x04: return "Controlador ISDN";
				case 0x07: return "Controlador InfiniBand";
				case 0x08: return "Controlador Fabric";
				case 0x80: return "Rede (outro)";
				default: return "Rede (desconhecido)";
			}

		case 0x03:
			switch (subclass)
			{
				case 0x00: return "Controlador VGA";
				case 0x01: return "Controlador XGA";
				case 0x02: return "Controlador 3D";
				case 0x80: return "Display (outro)";
				default: return "Display (desconhecido)";
			}

		case 0x04:
			switch (subclass)
			{
				case 0x00: return "Dispositivo de video";
				case 0x01: return "Dispositivo de audio";
				case 0x02: return "Dispositivo de telefonia";
				case 0x03: return "Controlador de audio";
				case 0x80: return "Multimidia (outro)";
				default: return "Multimidia (desconhecido)";
			}

		case 0x05:
			switch (subclass)
			{
				case 0x00: return "Controlador de RAM";
				case 0x01: return "Controlador de flash";
				case 0x80: return "Memoria (outro)";
				default: return "Memoria (desconhecido)";
			}

		case 0x06:
			switch (subclass)
			{
				case 0x00: return "Host bridge";
				case 0x01: return "Bridge ISA";
				case 0x04: return "Bridge PCI-PCI";
				case 0x07: return "Bridge CardBus";
				case 0x80: return "Bridge (outro)";
				default: return "Bridge (desconhecido)";
			}

		case 0x07:
			switch (subclass)
			{
				case 0x00: return "Controlador serial";
				case 0x01: return "Controlador paralelo";
				case 0x02: return "Controlador serial multiporta";
				case 0x03: return "Modem";
				case 0x80: return "Comunicacao (outro)";
				default: return "Comunicacao (desconhecido)";
			}

		case 0x08:
			switch (subclass)
			{
				case 0x00: return "Controlador de interrupcao (PIC)";
				case 0x01: return "Controlador DMA";
				case 0x02: return "Timer";
				case 0x03: return "RTC";
				case 0x80: return "Periferico de sistema (outro)";
				default: return "Periferico de sistema (desconhecido)";
			}

		case 0x09:
			switch (subclass)
			{
				case 0x00: return "Controlador de teclado";
				case 0x01: return "Caneta digital";
				case 0x02: return "Controlador de mouse";
				case 0x04: return "Controlador de gameport";
				case 0x80: return "Dispositivo de entrada (outro)";
				default: return "Dispositivo de entrada (desconhecido)";
			}

		case 0x0C:
			switch (subclass)
			{
				case 0x00: return "Controlador FireWire";
				case 0x03: return "Controlador USB";
				case 0x05: return "Controlador SMBus";
				case 0x80: return "Barramento serial (outro)";
				default: return "Barramento serial (desconhecido)";
			}

		case 0x0D:
			switch (subclass)
			{
				case 0x11: return "Controlador Bluetooth";
				case 0x12: return "Controlador de banda larga";
				case 0x80: return "Wireless (outro)";
				default: return "Wireless (desconhecido)";
			}

		case 0xFF:
			return "Classe especifica do fabricante";

		default:
			return "Classe desconhecida";
	}
}

/* Enumera todos os PCI */
void pci_enumerate(void)
{
	for (uint16_t bus = 0; bus < 256; bus++)
	{
		for (uint8_t dev = 0; dev < 32; dev++)
		{
			for (uint8_t func = 0; func < 8; func++)
			{
				uint32_t id = pci_read(bus, dev, func, 0);
				if ((id & 0xFFFF) == 0xFFFF)
				{
					if (func == 0)
						break;
					continue;
				}

				uint16_t vendor_id = id & 0xFFFF;
				uint16_t device_id = (id >> 16) & 0xFFFF;

				pci_device_t device;
				uint32_t reg2 = pci_read(bus, dev, func, 2);
				uint8_t class = (reg2 >> 24) & 0xFF;
				uint8_t subclass = (reg2 >> 16) & 0xFF;
				device.bus = bus;
				device.dev = dev;
				device.func = func;
				device.class = class;
				device.subclass = subclass;
				device.vendor_id = vendor_id;
				device.device_id = device_id;
				device.prog_if = (reg2 >> 8) & 0xFF;

				uint32_t reg3 = pci_read(bus, dev, func, 3);
				uint8_t header_type =
					(reg3 >> 16) & 0x7F; /* Ignora o bit de multifunção */
				if (header_type == 0x00)
				{
					for (int i = 0; i < 6; i++)
						device.bars[i] = pci_read(bus, dev, func, 4 + i);
				}
				else
				{
					for (int i = 0; i < 2; i++)
						device.bars[i] = pci_read(bus, dev, func, 4 + i);
				}

				pci_devices[pci_device_count++] = device;

				debugf("PCI: %04X %02hX:%02hhX.%hhu - %s\r\n", vendor_id, bus, dev, func,
						pci_get_class_string(class, subclass));
			}
		}
	}
}
