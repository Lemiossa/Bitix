#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <asm.h>
#include <pci.h>
#include <terminal.h>
#include <ata.h>

#define ATA_DATA 0
#define ATA_SECTOR_COUNT 2
#define ATA_LBA_LOW 3
#define ATA_LBA_MID 4
#define ATA_LBA_HIGH 5
#define ATA_DRIVE_HEAD 6
#define ATA_STATUS 7
#define ATA_COMMAND 7

#define ATA_STATUS_BSY (1 << 7)
#define ATA_STATUS_DRDY (1 << 6)
#define ATA_STATUS_DRQ (1 << 3)
#define ATA_STATUS_ERR (1 << 0)

ata_disk_t ata_disks[4];
int ata_disk_count = 0;

uint16_t base0 = 0;
uint16_t ctrl0 = 0;
uint16_t base1 = 0;
uint16_t ctrl1 = 0;

/* Espera um tempo pro ATA */
bool ata_wait(uint16_t base)
{
	uint8_t val = 0;
	for (int i = 0; i < 15; i++)
		val = inb(base + ATA_STATUS);
	return val & ATA_STATUS_BSY;
}

/* Retorna a porta ATA de acordo com uma BAR do PCI */
uint16_t ata_get_port(uint32_t bar, uint16_t legacy)
{
	if (bar == 0 || bar == 1)
		return legacy;
	return (uint16_t)(bar & 0xFFFC);
}

/* Comando identify do ATA */
/* retorna 0 = não existe, 1 = ATA, 2 = ATAPI */
int ata_identify(int disk, uint16_t *disk_info)
{
	if (disk > 3)
		return 0;

	uint16_t base = 0;
	if (disk >= 2) {
		base = base1;
		disk -= 2;
	} else {
		base = base0;
	}

	outb(base + ATA_DRIVE_HEAD, disk == 1 ? 0xB0 : 0xA0);
	outb(base + ATA_SECTOR_COUNT, 0);
	outb(base + ATA_LBA_LOW, 0);
	outb(base + ATA_LBA_MID, 0);
	outb(base + ATA_LBA_HIGH, 0);
	outb(base + ATA_COMMAND, 0xEC);

	uint8_t status = inb(base0 + ATA_STATUS);
	if (status == 0)
		return 0;

	if (ata_wait(base))
		return false;

	bool atapi = false;

	uint8_t lba_mid = inb(base + ATA_LBA_MID);
	uint8_t lba_high = inb(base + ATA_LBA_HIGH);
	if (lba_mid != 0 || lba_high != 0)
		atapi = true;

	do {
    	status = inb(base + ATA_STATUS);
	} while (!(status & ATA_STATUS_DRQ));

	uint16_t *info = (uint16_t *)disk_info;
	for (int i = 0; i < 256; i++) {
		info[i] = inw(base + ATA_DATA);
	}

	return atapi ? 2 : 1;
}

/* Lê N setores a partir de S setor de D disco em DST */
bool ata_read(int id, uint32_t s, uint8_t n, void *dst)
{
	if (!dst || id > 3)
		return false;

	ata_disk_t *d = &ata_disks[id];

	while (inb(d->base + ATA_STATUS) & ATA_STATUS_BSY);

	outb(d->base + ATA_DRIVE_HEAD,
			0xE0 | (d->slave << 4) | ((s >> 24) & 0x0F));

	if (ata_wait(d->base))
		return false;

	outb(d->base + ATA_SECTOR_COUNT, n);
	outb(d->base + ATA_LBA_LOW, s & 0xFF);
	outb(d->base + ATA_LBA_MID, (s >> 8) & 0xFF);
	outb(d->base + ATA_LBA_HIGH, (s >> 16) & 0xFF);

	outb(d->base + ATA_COMMAND, 0x20);

	uint16_t *buf = (uint16_t *)dst;
	for (uint8_t i = 0; i < n; i++) {
		uint8_t status;
		do {
			status = inb(d->base + ATA_STATUS);
			if (status & ATA_STATUS_ERR)
				return false;
		} while ((status & ATA_STATUS_BSY) || !(status & ATA_STATUS_DRQ));

		for (int j = 0; j < 256; j++) {
			buf[(i * 256) + j] = inw(d->base + ATA_DATA);
		}
	}

	return true;
}

/* Detecta os discos ATA */
bool ata_detect(void)
{
	pci_device_t *dev = pci_find(1, 1);
	if (!dev) {
		return false;
	}

	base0 = ata_get_port(dev->bars[0], 0x1F0);
	ctrl0 = ata_get_port(dev->bars[1], 0x3F6);

	base1 = ata_get_port(dev->bars[2], 0x170);
	ctrl1 = ata_get_port(dev->bars[3], 0x376);

	for (int i = 0; i < 4; i++) {
		uint16_t info[256] = {0};
		int type = ata_identify(i, info);
		if (type != 0) {
			ata_disk_t disk = {0};

			for (int j = 0; j < 20; j++) {
				disk.model[j * 2]     = (info[27 + j] >> 8) & 0xFF;
				disk.model[j * 2 + 1] = (info[27 + j]) & 0xFF;
			}

			disk.total_sectors = info[60] | ((uint32_t)info[61] << 16);

			for (int j = 0; j < 10; j++) {
				disk.serial[j * 2]     = (info[10 + j] >> 8) & 0xFF;
				disk.serial[j * 2 + 1] = (info[10 + j]) & 0xFF;
			}

			if (i >= 2) {
				disk.base = base1;
				disk.slave = (i - 2) == 1 ? true : false;
			} else {
				disk.base = base0;
				disk.slave = i == 1 ? true : false;
			}

			disk.atapi = type == 2 ? true : false;

			ata_disks[ata_disk_count++] = disk;
		}
	}

	return true;
}

