/************************************
 * fat.h                            *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#ifndef FAT_H
#define FAT_H
#include <stddef.h>
#include <stdint.h>

typedef struct fat_bpb
{
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

typedef union fat_ebpb
{
	struct
	{
		uint8_t drive_number;
		uint8_t nt_flags;
		uint8_t signature;
		uint32_t serial_number;
		uint8_t volume_label[11];
		uint8_t system_id[8];
	} _16;

	struct
	{
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

typedef struct bootsector
{
	fat_bpb_t bpb;
	fat_ebpb_t ebpb;
	uint8_t boot_code[420];
	uint16_t boot_signature;
} __attribute__((packed)) bootsector_t;

typedef struct fat_entry
{
	char name[11];
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

typedef struct fat_lfn_entry {
	uint8_t  order;
	uint16_t name1[5];
	uint8_t  attr;
	uint8_t  type;
	uint8_t  checksum;
	uint16_t name2[6];
	uint16_t cluster;
	uint16_t name3[2];
} __attribute__((packed)) fat_lfn_entry_t;

typedef struct fat_data
{
	uint32_t sectors_per_fat;
	uint32_t total_sectors;
	uint32_t root_dir_sectors;
	uint32_t data_sectors;
	uint32_t root_lba;
	uint32_t data_lba;
	uint32_t fat_lba;
	uint32_t total_clusters;
	uint8_t fat_type;
	uint8_t current_disk;
	bootsector_t bootsector;
} fat_data_t;

#define FAT_ATTR_RDOLY 0x01
#define FAT_ATTR_HIDDN 0x02
#define FAT_ATTR_SYSTM 0x04
#define FAT_ATTR_VOLID 0x08
#define FAT_ATTR_DIR 0x10
#define FAT_ATTR_ARCHV 0x20

bool fat_registry(int disk, uint32_t start_lba, char letter);

#endif /* FAT_H */
