/**
 * disk.c
 * Created by Matheus Leme Da Silva
 */
#include "disk.h"
#include "types.h"
#include "x86.h"

uchar buffer[BLOCK];

/**
 * Initialize disk
 */
void
init_disk (uchar drive)
{
	io_init_disk (drive);
	switch (drive) {
	case 0x00: {
		disk.label[0] = 'f';
		disk.label[1] = 'd';
		disk.label[2] = '0';
		disk.label[3] = 0;
	} break;
	case 0x01: {
		disk.label[0] = 'f';
		disk.label[1] = 'd';
		disk.label[2] = '1';
		disk.label[3] = 0;
	} break;
	case 0x80: {
		disk.label[0] = 'h';
		disk.label[1] = 'd';
		disk.label[2] = '0';
		disk.label[3] = 0;
	} break;
	case 0x81: {
		disk.label[0] = 'h';
		disk.label[1] = 'd';
		disk.label[2] = '1';
		disk.label[3] = 0;
	} break;
	default: {
		disk.label[0] = 'u';
		disk.label[1] = 'n';
		disk.label[2] = 'k';
		disk.label[3] = 0;
	} break;
	}
}

/**
 * Read block using CHS
 */
int
readblock (ushort lba, void *buf)
{
	ushort spt = disk.spt;
	ushort heads = disk.heads;
	ushort cylinder = lba / (spt * heads);
	ushort temp = lba % (spt * heads);
	uchar head = temp / spt;
	ushort sector = (temp % spt) + 1;
	int i, result;

	for (i = 0; i <= MAX_DISK_RETRIES; i++) {
		result = io_readblock_chs (head, cylinder, sector, buf);
		if (result)
			reset_disk ();
		else
			break;
	}

	return result;
}

/**
 * Write block using CHS
 */
int
writeblock (ushort lba, void *buf)
{
	ushort spt = disk.spt;
	ushort heads = disk.heads;
	ushort cylinder = lba / (spt * heads);
	ushort temp = lba % (spt * heads);
	uchar head = temp / spt;
	ushort sector = (temp % spt) + 1;
	int i, result;

	for (i = 0; i <= MAX_DISK_RETRIES; i++) {
		result = io_writeblock_chs (head, cylinder, sector, buf);
		if (result)
			reset_disk ();
		else
			break;
	}

	return result;
}

/**
 * Read far block
 */
int
freadblock (ushort lba, ushort seg, ushort off)
{
	int i;
	memset (buffer, 0, BLOCK);
	readblock (lba, buffer);
#ifdef DEBUG
	kputsf ("Reading lba %u %{0}4x:%{0}4x\n", lba, seg, off);
#endif
	for (i = 0; i < BLOCK; i++) {
		lwriteb (seg, off + i, buffer[i]);
	}
}

/**
 * Write far block
 */
int
fwriteblock (ushort lba, ushort seg, ushort off)
{
	int i;
	memset (buffer, 0, BLOCK);
#ifdef DEBUG
	kputsf ("Writing %{0}4x:%{0}4x to lba %u\n", seg, off, lba);
#endif
	lmemcpy (buffer, seg, off, 512);
	writeblock (lba, buffer);
}
