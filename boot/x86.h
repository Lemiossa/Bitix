#pragma once

#include "types.h"

extern uint8_t drive;

void io_putc(char c);
uint8_t io_disk_reset();
uint8_t io_readsectors(uint64_t lba, uint16_t seg, uint16_t off, uint16_t sectors);
