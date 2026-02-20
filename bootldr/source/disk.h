/************************************
 * disk.h                           *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#ifndef DISK_H
#define DISK_H
#include <stdint.h>

#define SECTOR_SIZE 512
extern uint8_t boot_drive;

uint8_t disk_get_parameters(uint8_t drive, uint16_t *cyl, uint8_t *hds, uint8_t *spt);
uint8_t disk_reset(uint8_t drive);
uint8_t disk_read_sector(uint8_t drive, void *dest, uint32_t lba);

#endif /* DISK_H */
