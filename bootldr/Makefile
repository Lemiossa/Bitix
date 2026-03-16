####################################
# Makefile                         #
# Criado por Matheus Leme Da Silva #
####################################

BUILDDIR ?= $(CURDIR)/build
BINDIR := $(BUILDDIR)/bootldr/bin
OBJDIR := $(BUILDDIR)/bootldr/obj
DEPDIR := $(BUILDDIR)/bootldr/dep
LIBDIR := $(BUILDDIR)/bootldr/lib

ARCH ?= x86

TARGET ?= $(BINDIR)/bootldr.bin

SOURCE := \
		  source/entry.asm \
		  source/main.c \
		  source/disk.c \
		  source/vesa.c \
		  source/util.c \
		  source/vga.c \
		  source/fat.c

SOURCE := $(addprefix $(CURDIR)/source/,$(SOURCE))

SOURCE_ASM := $(filter %.asm,$(SOURCE))
SOURCE_C := $(filter %.c,$(SOURCE))

OBJ := $(patsubst $(OBJDIR)/asm/%.o,$(CURDIR)/source/%.asm,$(SOURCE_ASM)) \
	   $(patsubst $(OBJDIR)/c/%.o,$(CURDIR)/source/%.c,$(SOURCE_C))

DEP := $(patsubst $(DEPDIR)/asm/%.d,$(CURDIR)/source/%.asm,$(SOURCE_ASM)) \
	   $(patsubst $(DEPDIR)/c/%.d,$(CURDIR)/source/%.c,$(SOURCE_C))

.PHONY: all
all: image

.PHONY: clean
clean:
	rm -f $(TARGET) $(OBJ) $(DEP)

-include $(DEP)
