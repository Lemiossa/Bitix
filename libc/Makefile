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
ARCH ?= x86

INCLUDES ?= -I$(CURDIR)/include
BINDIR := $(addsuffix /libc,$(BINDIR))
OBJDIR := $(addsuffix /libc,$(OBJDIR))
DEPDIR := $(addsuffix /libc,$(DEPDIR))

NASM := nasm
CC ?= gcc
AR ?= ar

ifeq ($(ARCH),x86)
	NASMFLAGS := -f elf32
	CFLAGS := -m32 -march=i686
else
	$(error Arquitetura não suportada: $(ARCH))
endif

CFLAGS += -std=c99 -Wall -Wextra -O2 \
		  -fno-PIC -fno-pie -fno-stack-protector \
		  -fno-stack-check -ffreestanding -fno-lto -fno-builtin \
		  -ffunction-sections -fdata-sections -mno-mmx -mno-sse \
		  -mno-sse2 -fno-common -I$(SOURCEDIR)/arch/$(ARCH) $(INCLUDES)
LDFLAGS := rcv

TARGET ?= $(LIBDIR)/libc.a

SOURCE := \
		  string.c \
		  stdio.c
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

$(TARGET): $(OBJ)
	mkdir -p $(dir $@)
	$(AR) $(ARFLAGS) $@ $^

$(OBJDIR)/c/%.o: $(SOURCEDIR)/%.c
	mkdir -p $(dir $@) $(dir $(DEPDIR)/c/$*.d)
	$(CC) $(CFLAGS) -c $< -o $@ -MD -MP -MF $(DEPDIR)/c/$*.d

$(OBJDIR)/asm/%.o: $(SOURCEDIR)/%.asm
	mkdir -p $(dir $@) $(dir $(DEPDIR)/asm/$*.d)
	$(NASM) $(NASMFLAGS) $< -o $@ -MD -MP -MF $(DEPDIR)/asm/$*.d

-include $(DEP)
