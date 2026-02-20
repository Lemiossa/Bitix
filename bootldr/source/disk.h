/************************************
 * disk.h                           *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#ifndef DISK_H
#define DISK_H
#include <stdint.h>

#define MAX_DISKS 18
#define SECTOR_SIZE 512
extern int boot_disk;
extern uint8_t boot_drive;

uint8_t disk_get_parameters(int disk, uint16_t *cylinders, uint8_t *heads, uint8_t *sectors);
uint8_t disk_reset(int disk);
uint8_t disk_read_sector(int disk, void *dest, uint32_t lba);
int disk_find_letter(char letter);
int disk_find_drive(uint8_t drive);
char disk_get_letter(int disk);
void disk_dettect(void);

#endif /* DISK_H */
