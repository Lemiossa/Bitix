#
# kernel/Makefile
# Created by Matheus Leme Da Silva
#

# compilers
CC = gcc
LD = ld
AS = nasm

ASFLAGS = -f elf32
CFLAGS = -m32 -ffreestanding -nostdlib -Wall -Wextra -Werror -fno-pie -fno-pic -I. -O0
LDFLAGS = -m elf_i386 -T linker.ld -nostdlib 
TARGET ?= kernel.bin

.PHONY: all clean

all: $(TARGET)

clean:
	rm -f *.bin *.o $(TARGET)

$(TARGET): i386/entry.o kernel.o libgcc.o console.o 
	$(LD) $(LDFLAGS) -o $@ i386/entry.o kernel.o libgcc.o console.o
 
i386/entry.o: i386/entry.asm
	$(AS) $(ASFLAGS) $< -o $@

console.o: console.c 
	$(CC) $(CFLAGS) -c $< -o $@

libgcc.o: libgcc.c 
	$(CC) $(CFLAGS) -c $< -o $@

kernel.o: kernel.c 
	$(CC) $(CFLAGS) -c $< -o $@
