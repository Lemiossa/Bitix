/**
 * disk.c
 * Created by Matheus Leme Da Silva
 */
#include "types.h"
#include "x86.h"
#include "ports.h"
#include "util.h"
#include "disk.h"

#define DISK_READ_MAX_RETRIES 3

uint8_t readsectors(uint64_t lba, void *buf, uint16_t sectors)
{
	uint8_t res=0;

	for(int i=0;i<DISK_READ_MAX_RETRIES;i++) {
		res=io_readsectors(lba, 0x8000, 0x0000, sectors);
		if(!res) {
			break;
		}
		printf("Failed to read LBA %llu\n", lba);
		io_disk_reset();
	}

	if(res!=0) {
		Sprintf("Failed to read sector %llu (ERROR CODE: %hhx)\n", lba, res);
		for(;;);
	}

	memcpy(buf, (void*)0x80000, sectors*512);
	return 0;
}
