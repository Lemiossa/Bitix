####################################
# Makefile                         #
# Criado por Matheus Leme Da Silva #
####################################

SOURCEDIR := $(CURDIR)/source
BUILDDIR ?= $(CURDIR)/build
BINDIR ?= $(BUILDDIR)/bin
OBJDIR ?= $(BUILDDIR)/obj
DEPDIR ?= $(BUILDDIR)/dep
LIBDIR ?= $(BUILDDIR)/lib

BINDIR := $(addsuffix /boot,$(BINDIR))
OBJDIR := $(addsuffix /boot,$(OBJDIR))
DEPDIR := $(addsuffix /boot,$(DEPDIR))

INCLUDES ?= -I$(CURDIR)/include

LINKER_FILE := $(SOURCEDIR)/linker.ld

NASM := nasm
NASMFLAGS := -f elf32

CC ?= gcc
LD ?= ld

CFLAGS := -m32 -std=c99 -Wall -Wextra -Os \
		  -fno-PIC -fno-pie -fno-stack-protector -march=i686 \
		  -fno-stack-check -ffreestanding -fno-lto -fno-builtin \
		  -ffunction-sections -fdata-sections -mno-mmx -mno-sse \
		  -mno-sse2 -fno-common $(INCLUDES)
LDFLAGS := -m elf_i386 -nostdlib -static --gc-sections -T $(LINKER_FILE)

TARGET ?= $(BINDIR)/bootldr.bin
LIBS := $(LIBDIR)/libc.a

SOURCE := \
		  entry.asm \
		  vga.c \
		  fat.c \
		  e820.c \
		  disk.c \
		  util.c \
		  file.c \
		  main.c
SOURCE := $(addprefix $(SOURCEDIR)/,$(SOURCE))

SOURCEC := $(filter %.c,$(SOURCE))
SOURCEASM := $(filter %.asm,$(SOURCE))

OBJC := $(patsubst $(SOURCEDIR)/%.c,$(OBJDIR)/c/%.o,$(SOURCEC))
OBJASM := $(patsubst $(SOURCEDIR)/%.asm,$(OBJDIR)/asm/%.o,$(SOURCEASM))

OBJ := $(OBJASM) $(OBJC)

DEPC := $(patsubst $(SOURCEDIR)/%.c,$(DEPDIR)/c/%.d,$(SOURCEC))
DEPASM := $(patsubst $(SOURCEDIR)/%.asm,$(DEPDIR)/asm/%.d,$(SOURCEASM))

DEP := $(DEPASM) $(DEPC) $(DEPDIR)/asm/bootsect.d

.PHONY: all
all: $(TARGET)

.PHONY: clean
clean:
	rm -f $(BINDIR)/bootsect.bin $(TARGET) $(DEP) $(OBJ)

$(TARGET): $(BINDIR)/bootsect.bin $(BINDIR)/loader.bin
	mkdir -p $(dir $@)
	cat $^ > $@

$(BINDIR)/bootsect.bin: $(SOURCEDIR)/bootsect.asm
	mkdir -p $(dir $@) $(dir $(DEPDIR)/asm/bootsect.d)
	$(NASM) -f bin $< -o $@ -MD -MP -MF $(DEPDIR)/asm/bootsect.d

$(BINDIR)/loader.elf: $(OBJ) $(LIBS)
	mkdir -p $(dir $@)
	$(LD) $(LDFLAGS) -o $@ $^

$(BINDIR)/loader.bin: $(BINDIR)/loader.elf
	mkdir -p $(dir $@)
	objcopy -O binary $< $@

$(OBJDIR)/c/%.o: $(SOURCEDIR)/%.c
	mkdir -p $(dir $@) $(dir $(DEPDIR)/c/$*.d)
	$(CC) $(CFLAGS) -c $< -o $@ -MD -MP -MF $(DEPDIR)/c/$*.d

$(OBJDIR)/asm/%.o: $(SOURCEDIR)/%.asm
	mkdir -p $(dir $@) $(dir $(DEPDIR)/asm/$*.d)
	$(NASM) $(NASMFLAGS) $< -o $@ -MD -MP -MF $(DEPDIR)/asm/$*.d

-include $(DEP)
