#
# Makefile
# Created by Matheus Leme Da Silva
#

ifndef DEBUG
    DEBUG=false
endif
ifndef BIND
    BIND=build/binaries
endif
ifndef OBJD
    OBJD=build/objects
endif

AS=nasm
LD=ld86
CC=bcc

ASFLAGS86=-f bin
ASFLAGS=-f as86
CFLAGS=-O -0 -Md -ansi -W -D__STANDALONE__=1 -Dconst= -I.
LDFLAGS=-d -s -L -T0x0000

ifeq ($(DEBUG),true)
LDFLAGS+=-M -N
CFLAGS+=
endif

SOURCES=\
		entry.asm \
		x86.asm \
		runtime.asm \
		utils.c \
		main.c

OBJECTS=$(patsubst %.asm,$(OBJD)/%.o,$(patsubst %.c,$(OBJD)/%.o,$(SOURCES)))

TARGET?=boot.bin

.PHONY: all clean

all: mbr.bin $(TARGET)

clean:
	@echo "  RM $(OBJECTS) $(TARGET)"
	@rm -f $(OBJECTS) $(TARGET)

mbr.bin: mbr.asm
	@$(AS) $(ASFLAGS86) $< -o $@

$(TARGET): $(OBJECTS)
	@echo "  LD $@"
	@mkdir -p $(dir $@)
	@$(LD) $(LDFLAGS) -o bootloader.bin $(OBJECTS)
	@cat mbr.bin bootloader.bin > $@
	@truncate -s 16K $@

$(OBJD)/%.o: %.asm
	@echo "  AS $@"
	@mkdir -p $(dir $@)
	@$(AS) $(ASFLAGS) $< -o $@

$(OBJD)/%.o: %.c
	@echo "  CC $@"
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c $< -o $@
