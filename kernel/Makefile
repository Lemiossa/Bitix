####################################
# Makefile                         #
# Criado por Matheus Leme Da Silva #
####################################

SOURCEDIR := $(CURDIR)/source
BUILDDIR ?= $(CURDIR)/build
BINDIR ?= $(BUILDDIR)/bin
OBJDIR ?= $(BUILDDIR)/obj
DEPDIR ?= $(BUILDDIR)/dep
ARCH ?= x86

BINDIR := $(addsuffix /kernel,$(BINDIR))
OBJDIR := $(addsuffix /kernel,$(OBJDIR))
DEPDIR := $(addsuffix /kernel,$(DEPDIR))

LINKER_FILE := $(SOURCEDIR)/arch/$(ARCH)/linker.ld

NASM := nasm
CC ?= gcc
LD ?= ld

ifeq ($(ARCH),x86)
	NASMFLAGS := -f elf32
	CFLAGS := -m32 -march=i686
else
	$(error Arquitetura não suportada: $(ARCH))
endif

CFLAGS += -std=c99 -Wall -Wextra -O0 \
		  -fno-PIC -fno-pie -fno-stack-protector \
		  -fno-stack-check -ffreestanding -fno-lto -fno-builtin \
		  -ffunction-sections -fdata-sections -mno-mmx -mno-sse \
		  -mno-sse2 -fno-common
LDFLAGS := -m elf_i386 -nostdlib -static --gc-sections -T $(LINKER_FILE)

TARGET ?= $(BINDIR)/kernel.bin

SOURCE := \
		  arch/$(ARCH)/entry.asm \
		  main.c
SOURCE := $(addprefix $(SOURCEDIR)/,$(SOURCE))

SOURCEC := $(filter %.c,$(SOURCE))
SOURCEASM := $(filter %.asm,$(SOURCE))

OBJC := $(patsubst $(SOURCEDIR)/%.c,$(OBJDIR)/c/%.o,$(SOURCEC))
OBJASM := $(patsubst $(SOURCEDIR)/%.asm,$(OBJDIR)/asm/%.o,$(SOURCEASM))

OBJ := $(OBJASM) $(OBJC)

DEPC := $(patsubst $(SOURCEDIR)/%.c,$(DEPDIR)/c/%.d,$(SOURCEC))
DEPASM := $(patsubst $(SOURCEDIR)/%.asm,$(DEPDIR)/asm/%.d,$(SOURCEASM))

DEP := $(DEPASM) $(DEPC)

.PHONY: all
all: $(TARGET)

.PHONY: clean
clean:
	rm -f $(TARGET) $(DEP) $(OBJ)

$(TARGET): $(BINDIR)/kernel.elf
	mkdir -p $(dir $@)
	objcopy -O binary $< $@

$(BINDIR)/kernel.elf: $(OBJ)
	mkdir -p $(dir $@)
	$(LD) $(LDFLAGS) -o $@ $^

$(OBJDIR)/c/%.o: $(SOURCEDIR)/%.c
	mkdir -p $(dir $@) $(dir $(DEPDIR)/c/$*.d)
	$(CC) $(CFLAGS) -c $< -o $@ -MD -MP -MF $(DEPDIR)/c/$*.d

$(OBJDIR)/asm/%.o: $(SOURCEDIR)/%.asm
	mkdir -p $(dir $@) $(dir $(DEPDIR)/asm/$*.d)
	$(NASM) $(NASMFLAGS) $< -o $@ -MD -MP -MF $(DEPDIR)/asm/$*.d

-include $(DEP)
