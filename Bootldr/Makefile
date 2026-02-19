####################################
# Makefile                         #
# Criado por Matheus Leme Da Silva #
####################################

SOURCEDIR := $(CURDIR)/Source
BUILDDIR ?= $(CURDIR)/Build
BINDIR ?= $(BUILDDIR)/Bin
OBJDIR ?= $(BUILDDIR)/Obj
DEPDIR ?= $(BUILDDIR)/Dep

BINDIR := $(addsuffix /Boot,$(BINDIR))
OBJDIR := $(addsuffix /Boot,$(OBJDIR))
DEPDIR := $(addsuffix /Boot,$(DEPDIR))

LINKER_FILE := $(SOURCEDIR)/Linker.ld

NASM := nasm
NASMFLAGS := -f elf32

CC ?= clang
LD ?= ld.lld

CFLAGS := -target i686-elf -Wall -Wextra -m32 -std=c99 -Os \
		  -fno-PIC -fno-pie -fno-stack-protector -march=i686 \
		  -fno-stack-check -ffreestanding -fno-lto -mno-red-zone \
		  -ffunction-sections -fdata-sections -mno-mmx -mno-sse \
		  -mno-sse2
LDFLAGS := -m elf_i386 -nostdlib -static --gc-sections -T $(LINKER_FILE)

TARGET ?= $(BINDIR)/Bootldr.bin

SOURCE := \
		  Entry.asm \
		  Main.c
SOURCE := $(addprefix $(SOURCEDIR)/,$(SOURCE))

SOURCEC := $(filter %.c,$(SOURCE))
SOURCEASM := $(filter %.asm,$(SOURCE))

OBJC := $(patsubst $(SOURCEDIR)/%.c,$(OBJDIR)/C/%.o,$(SOURCEC))
OBJASM := $(patsubst $(SOURCEDIR)/%.asm,$(OBJDIR)/Asm/%.o,$(SOURCEASM))

OBJ := $(OBJASM) $(OBJC)

DEPC := $(patsubst $(SOURCEDIR)/%.c,$(DEPDIR)/C/%.d,$(SOURCEC))
DEPASM := $(patsubst $(SOURCEDIR)/%.asm,$(DEPDIR)/Asm/%.d,$(SOURCEASM))

DEP := $(DEPASM) $(DEPC) $(DEPDIR)/Asm/Bootsect.d

.PHONY: all
all: $(TARGET)

.PHONY: clean
clean:
	rm -f $(BINDIR)/Bootsect.bin $(TARGET) $(DEP) $(OBJ)

$(TARGET): $(BINDIR)/Bootsect.bin $(BINDIR)/Loader.bin
	mkdir -p $(dir $@)
	cat $^ > $@

$(BINDIR)/Bootsect.bin: $(SOURCEDIR)/Bootsect.asm
	mkdir -p $(dir $@) $(dir $(DEPDIR)/Asm/Bootsect.d)
	$(NASM) -f bin $< -o $@ -MD -MP -MF $(DEPDIR)/Asm/Bootsect.d

$(BINDIR)/Loader.elf: $(OBJ)
	mkdir -p $(dir $@)
	$(LD) $(LDFLAGS) -o $@ $^

$(BINDIR)/Loader.bin: $(BINDIR)/Loader.elf
	mkdir -p $(dir $@)
	objcopy -O binary $< $@

$(OBJDIR)/C/%.o: $(SOURCEDIR)/%.c
	mkdir -p $(dir $@) $(dir $(DEPDIR)/C/$*.d)
	$(CC) $(CFLAGS) -c $< -o $@ -MD -MP -MF $(DEPDIR)/C/$*.d

$(OBJDIR)/Asm/%.o: $(SOURCEDIR)/%.asm
	mkdir -p $(dir $@) $(dir $(DEPDIR)/Asm/$*.d)
	$(NASM) $(NASMFLAGS) $< -o $@ -MD -MP -MF $(DEPDIR)/Asm/$*.d

-include $(DEP)
