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

# programs
AS=nasm
LD=ld86
CC=bcc
AR=ar86

BDATE=$(shell date +"%Y/%m/%d-%H:%M:%S")

ASFLAGS=-f as86
CFLAGS=-O -Md -ansi -W -D__STANDALONE__=1 -Dconst= -DKBUILDV=\"$(BUILDV)\" -DBUILD_DATE=\"$(BDATE)\" -I. -I./include
LDFLAGS=-d -s -L -T0x0000

-include ../.config.flags

ifeq ($(DEBUG),true)
LDFLAGS+=-M -N
CFLAGS+=
endif
SOURCES=\
				x86/entry.asm \
				x86/runtime.asm \
				x86/x86.asm \
				utils.c \
				kernel.c
OBJECTS=$(patsubst %.asm,$(OBJD)/%.o,$(patsubst %.c, $(OBJD)/%.o, $(SOURCES)))
KERNEL?=kernel.bin
BOOT?=boot.bin
.PHONY: all clean
all: $(BOOT) $(KERNEL)
$(OBJECTS): ../.config
clean:
	@echo "  RM $(OBJECTS) $(KERNEL)"
	@rm -f $(OBJECTS) $(KERNEL)
	@$(MAKE) -C boot clean BIND=$(BIND) OBJD=$(OBJD) DEBUG=$(DEBUG) TARGET=$(BOOT)
$(KERNEL): $(OBJECTS)
	@echo "  LD $@"
	@mkdir -p $(dir $@)
	@$(LD) $(LDFLAGS) -o $@ $(OBJECTS)
$(BOOT):
	@$(MAKE) -C boot BIND=$(BIND) OBJD=$(OBJD) DEBUG=$(DEBUG) TARGET=$(BOOT)

$(OBJD)/%.o: %.asm
	@echo "  AS $@"
	@mkdir -p $(dir $@)
	@$(AS) $(ASFLAGS) $< -o $@
$(OBJD)/%.o: %.c
	@echo "  CC $@"
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c $< -o $@
