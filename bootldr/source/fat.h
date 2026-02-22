/************************************
 * fat.h                            *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#ifndef FAT_H
#define FAT_H
#include <stdint.h>
#include <stddef.h>

typedef struct fat_bpb {
	uint8_t jmp[3];
	uint8_t oem_name[8];
	uint16_t bytes_per_sector;
	uint8_t sectors_per_cluster;
	uint16_t reserved_sectors;
	uint8_t num_fats;
	uint16_t root_dir_entries;
	uint16_t total_sectors16;
	uint8_t media_descriptor;
	uint16_t sectors_per_fat16;
	uint16_t sectors_per_track;
	uint16_t num_heads;
	uint32_t hidden_sectors;
	uint32_t total_sectors32;
} __attribute__((packed)) fat_bpb_t;

typedef union fat_ebpb {
	struct {
		uint8_t drive_number;
		uint8_t nt_flags;
		uint8_t signature;
		uint32_t serial_number;
		uint8_t volume_label[11];
		uint8_t system_id[8];
	} _16;

	struct {
		uint32_t sectors_per_fat32;
		uint16_t flags;
		uint8_t version_major;
		uint8_t version_minor;
		uint32_t root_dir_cluster;
		uint16_t fs_info_sector;
		uint16_t backup_sector;
		uint8_t reserved[12];
		uint8_t drive_number;
		uint8_t nt_flags;
		uint8_t signature;
		uint32_t serial_number;
		uint8_t volume_label[11];
		uint8_t system_id[8];
	} _32;
} __attribute__((packed)) fat_ebpb_t;

typedef struct bootsector {
	fat_bpb_t bpb;
	fat_ebpb_t ebpb;
	uint8_t boot_code[420];
	uint16_t boot_signature;
} __attribute__((packed)) bootsector_t;

typedef struct fat_entry {
	uint8_t name[11];
	uint8_t attr;
	uint16_t res;
	uint16_t ctime;
	uint16_t cdate;
	uint16_t adate;
	uint16_t cluster_high;
	uint16_t mtime;
	uint16_t mdate;
	uint16_t cluster_low;
	uint32_t file_size; /* Em bytes */
} __attribute__((packed)) fat_entry_t;

#define FAT_ATTR_RDOLY 0x01
#define FAT_ATTR_HIDDN 0x02
#define FAT_ATTR_SYSTM 0x04
#define FAT_ATTR_VOLID 0x08
#define FAT_ATTR_DIR   0x10
#define FAT_ATTR_ARCHV 0x20

void fat_name_to_filename(char *fatname, char *out);
void fat_filename_to_fatname(char *filename, char *out);
int fat_configure(int disk, uint32_t lba);
int fat_read_dir(uint32_t cluster, uint32_t index, fat_entry_t *out);
size_t fat_read(void *dest, fat_entry_t *entry, size_t offset, size_t n);

#endif /* FAT_H */
