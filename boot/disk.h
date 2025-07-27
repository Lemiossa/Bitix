#pragma once

#include "types.h"

void init_disk_controller();
uint8_t readsectors(uint64_t lba, void *buf, uint16_t sectors);
