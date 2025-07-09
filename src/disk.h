#ifndef DISK_H
#define DISK_H

#include "types.h"

#define MAX_DISK_RETRIES 3
#define BLOCK 512

void init_disk (uchar drive);
int readblock (ushort lba, void *buf);
int freadblock (ushort lba, ushort seg, ushort off);
int writeblock (ushort lba, void *buf);
int fwriteblock (ushort lba, ushort seg, ushort off);

#endif /* DISK_H */
