/************************************
 * ata.h                            *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#ifndef ATA_H
#define ATA_H
#include <stdbool.h>
#include <stdint.h>

typedef struct ata_disk
{
	char model[41];
	char serial[21];
	uint32_t total_sectors;
	uint16_t base;
	bool atapi;
	bool slave;
} ata_disk_t;

typedef struct mbr_part
{
	uint8_t status;
	uint8_t start_chs[3];
	uint8_t type;
	uint8_t end_chs[3];
	uint32_t start_lba;
	uint32_t length;
} __attribute__((packed)) mbr_part_t;

#define SECTOR_SIZE 512

extern ata_disk_t ata_disks[4];
extern int ata_disk_count;

bool ata_read(int id, uint32_t s, uint8_t n, void *dst);
bool ata_write(int id, uint32_t s, uint8_t n, void *src);
bool ata_detect(void);

#endif /* ATA_H */
