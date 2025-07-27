#
# boot/Makefile
# Created by Matheus Leme Da Silva
#

# Compilers and tools
CC = gcc
LD = ld
AS = nasm

ASFLAGS86 = -f bin

LINKER = linker.ld

ASFLAGS = -f elf32
CFLAGS = -m32 -ffreestanding -nostdlib -Wall -Wextra -Werror -fno-pie -fno-pic -I. -O0 -g0
LDFLAGS = -m elf_i386 -T $(LINKER) -nostdlib 

# Target and files
TARGET ?= bootloader.bin

.PHONY: all clean

all: $(TARGET)

clean:
	rm -f $(TARGET) *.bin *.o
	
mbr.bin: mbr.asm 
	$(AS) $(ASFLAGS86) $< -o $@

entry.o: entry.asm
	$(AS) $(ASFLAGS) $< -o $@

x86.o: x86.asm
	$(AS) $(ASFLAGS) $< -o $@

main.o: main.c
	$(CC) $(CFLAGS) -c $< -o $@

util.o: util.c
	$(CC) $(CFLAGS) -c $< -o $@

disk.o: disk.c
	$(CC) $(CFLAGS) -c $< -o $@

fs.o: fs.c
	$(CC) $(CFLAGS) -c $< -o $@

pci.o: pci.c
	$(CC) $(CFLAGS) -c $< -o $@

boot.bin: entry.o x86.o main.o util.o disk.o fs.o pci.o
	$(LD) $(LDFLAGS) -o $@ $^

$(TARGET): mbr.bin boot.bin
	cat mbr.bin boot.bin > $@
