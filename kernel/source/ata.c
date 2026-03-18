/************************************
 * ata.c                            *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#include <asm.h>
#include <ata.h>
#include <pci.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <fat.h>
#include <terminal.h>
#include <debug.h>

#define ATA_DATA 0
#define ATA_SECTOR_COUNT 2
#define ATA_LBA_LOW 3
#define ATA_LBA_MID 4
#define ATA_LBA_HIGH 5
#define ATA_DRIVE_HEAD 6
#define ATA_STATUS 7
#define ATA_COMMAND 7

#define ATA_STATUS_ERR (1 << 0)
#define ATA_STATUS_IDX (1 << 1)
#define ATA_STATUS_CORR (1 << 2)
#define ATA_STATUS_DRQ (1 << 3)
#define ATA_STATUS_SRV (1 << 4)
#define ATA_STATUS_DF (1 << 5)
#define ATA_STATUS_RDY (1 << 6)
#define ATA_STATUS_BSY (1 << 7)

ata_disk_t ata_disks[4];
int ata_disk_count = 0;

uint16_t base0 = 0;
uint16_t ctrl0 = 0;
uint16_t base1 = 0;
uint16_t ctrl1 = 0;

/* Espera um tempo pro ATA */
static inline bool ata_delay(uint16_t base)
{
	uint8_t val = 0;
	for (int i = 0; i < 15; i++)
		val = inb(base + ATA_STATUS);
	return val & ATA_STATUS_BSY;
}

/* Espera até o bit BSY ser limpo */
static inline bool ata_wait_bsy(uint16_t base)
{
	uint8_t status;
	do
	{
		status = inb(base + ATA_STATUS);
		if (status & (ATA_STATUS_DF | ATA_STATUS_ERR))
			return false;
	} while (status & ATA_STATUS_BSY);
	return true;
}

/* Espera até o bit DRQ estar ativo */
static inline bool ata_wait_drq(uint16_t base)
{
	uint8_t status;
	do
	{
		status = inb(base + ATA_STATUS);
		if (status & (ATA_STATUS_DF | ATA_STATUS_ERR))
			return false;
	} while (!(status & ATA_STATUS_DRQ));
	return true;
}

/* Comando identify do ATA */
/* retorna 0 = não existe, 1 = ATA, 2 = ATAPI */
int ata_identify(int disk, uint16_t *disk_info)
{
	if (disk > 3)
		return 0;

	uint16_t base = 0;
	if (disk >= 2)
	{
		base = base1;
		disk -= 2;
	}
	else
	{
		base = base0;
	}

	outb(base + ATA_DRIVE_HEAD, disk == 1 ? 0xB0 : 0xA0);
	ata_delay(base);

	outb(base + ATA_SECTOR_COUNT, 0);
	outb(base + ATA_LBA_LOW, 0);
	outb(base + ATA_LBA_MID, 0);
	outb(base + ATA_LBA_HIGH, 0);
	outb(base + ATA_COMMAND, 0xEC);

	uint8_t status = inb(base + ATA_STATUS);
	if (status == 0)
		return 0;

	if (ata_delay(base))
		return false;

	bool atapi = false;

	uint8_t lba_mid = inb(base + ATA_LBA_MID);
	uint8_t lba_high = inb(base + ATA_LBA_HIGH);
	if (lba_mid != 0 || lba_high != 0)
		atapi = true;

	if (!ata_wait_drq(base))
		return false;

	uint16_t *info = (uint16_t *)disk_info;
	for (int i = 0; i < 256; i++)
	{
		status = inb(base + ATA_STATUS);
		if (status & ATA_STATUS_ERR)
			return 0;

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

	if (!ata_wait_bsy(d->base))
		return false;

	outb(d->base + ATA_DRIVE_HEAD, 0xE0 | (d->slave << 4) | ((s >> 24) & 0x0F));

	if (ata_delay(d->base))
		return false;

	outb(d->base + ATA_SECTOR_COUNT, n);
	outb(d->base + ATA_LBA_LOW, s & 0xFF);
	outb(d->base + ATA_LBA_MID, (s >> 8) & 0xFF);
	outb(d->base + ATA_LBA_HIGH, (s >> 16) & 0xFF);

	outb(d->base + ATA_COMMAND, 0x20);

	uint16_t *buf = (uint16_t *)dst;
	for (uint8_t i = 0; i < n; i++)
	{
		if (!ata_wait_bsy(d->base))
			return false;

		if (!ata_wait_drq(d->base))
			return false;

		for (int j = 0; j < 256; j++)
		{
			buf[(i * 256) + j] = inw(d->base + ATA_DATA);
		}
	}

	return true;
}

/* Escreve N setores de src a partir de S setor de D disco */
bool ata_write(int id, uint32_t s, uint8_t n, void *src)
{
	if (!src || id > 3)
		return false;

	ata_disk_t *d = &ata_disks[id];

	if (!ata_wait_bsy(d->base))
		return false;

	outb(d->base + ATA_DRIVE_HEAD, 0xE0 | (d->slave << 4) | ((s >> 24) & 0x0F));

	if (ata_delay(d->base))
		return false;

	outb(d->base + ATA_SECTOR_COUNT, n);
	outb(d->base + ATA_LBA_LOW, s & 0xFF);
	outb(d->base + ATA_LBA_MID, (s >> 8) & 0xFF);
	outb(d->base + ATA_LBA_HIGH, (s >> 16) & 0xFF);

	outb(d->base + ATA_COMMAND, 0x30);

	uint16_t *buf = (uint16_t *)src;
	for (uint8_t i = 0; i < n; i++)
	{
		if (!ata_wait_bsy(d->base))
			return false;

		if (!ata_wait_drq(d->base))
			return false;

		for (int j = 0; j < 256; j++)
		{
			outw(d->base + ATA_DATA, buf[(i * 256) + j]);
		}
	}

	return true;
}

/* Detecta os discos ATA */
bool ata_detect(void)
{
	pci_device_t *dev = pci_find(1, 1);
	if (!dev)
	{
		return false;
	}

	if (dev->prog_if & 1)
	{ /* Se o canal principal está em modo PCI */
		base0 = dev->bars[0] & 0xFFFFFFFC;
		ctrl0 = dev->bars[1] & 0xFFFFFFFC;

		if (!base0)
			base0 = 0x1F0;
		if (!ctrl0)
			ctrl0 = 0x3F6;
	}
	else
	{
		base0 = 0x1F0;
		ctrl0 = 0x3F6;
	}

	if (dev->prog_if & 4)
	{ /* Se o canal secundário está em modo PCI */
		base1 = dev->bars[2] & 0xFFFFFFFC;
		ctrl1 = dev->bars[3] & 0xFFFFFFFC;
		if (!base1)
			base1 = 0x170;
		if (!ctrl1)
			ctrl1 = 0x376;
	}
	else
	{
		base1 = 0x170;
		ctrl1 = 0x376;
	}

	char letter = 'C';
	for (int i = 0; i < 4; i++)
	{
		uint16_t info[256] = {0};
		int type = ata_identify(i, info);
		if (type != 0)
		{
			ata_disk_t disk = {0};

			for (int j = 0; j < 20; j++)
			{
				disk.model[j * 2] = (info[27 + j] >> 8) & 0xFF;
				disk.model[j * 2 + 1] = (info[27 + j]) & 0xFF;
			}

			disk.total_sectors = info[60] | ((uint32_t)info[61] << 16);

			for (int j = 0; j < 10; j++)
			{
				disk.serial[j * 2] = (info[10 + j] >> 8) & 0xFF;
				disk.serial[j * 2 + 1] = (info[10 + j]) & 0xFF;
			}

			if (i >= 2)
			{
				disk.base = base1;
				disk.slave = (i - 2) == 1 ? true : false;
			}
			else
			{
				disk.base = base0;
				disk.slave = i == 1 ? true : false;
			}

			disk.model[40] = 0;
			disk.serial[20] = 0;
			disk.atapi = type == 2 ? true : false;

			ata_disks[ata_disk_count++] = disk;
			debugf("ATA%d:\r\n", i);
			debugf("\tModelo: %s\r\n", ata_disks[i].model);
			debugf("\tSerial: %s\r\n", ata_disks[i].serial);
			debugf("\tTamanho: %u b\r\n", ata_disks[i].total_sectors * 512);

			uint8_t mbr[SECTOR_SIZE] = {0};

			if (!ata_read(i, 0, 1, mbr))
				continue;

			mbr_part_t *parts = (mbr_part_t *)&mbr[0x1BE];
			int part_count = 0;
			for (int j = 0; j < 4; j++)
			{
				uint8_t type = parts[j].type;
				if (type == 0)
					continue;

				switch (type)
				{
					/* FAT */
					case 0x01:
					case 0x04:
					case 0x06:
					case 0x0E:
					case 0x0B:
					case 0x0C:
					case 0x1B:
					case 0x1C:
						if (fat_registry(i, parts[j].start_lba, letter))
						{
							debugf("ATA%d: Particao FAT: \"%c:\"\r\n", i, letter);
							part_count++;
							letter++;
						}
						break;
					default:
						debugf("ATA%d: Tipo de particao desconhecido: 0x%02hhX\r\n", i, type);
						break;
				}
			}

			if (part_count == 0)
			{
				debugf("ATA%d: Numero de particoes e 0, tentando usar setor 0...\r\n", i);
				if (fat_registry(i, 0, letter))
				{
					debugf("ATA%d: Sucesso! Criado como particao FAT: \"%c:\"\r\n", i, letter);
					letter++;
				}
				else
				{
					debugf("ATA%d: Erro!\r\n", i);
				}
			}
		}
	}
	return true;
}
