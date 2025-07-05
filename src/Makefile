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
LDFLAGS=-d -s -L

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

PROGRAMS=\
	../programs/shell.c

OBJECTS=$(patsubst %.asm,$(OBJD)/%.o,$(patsubst %.c, $(OBJD)/%.o, $(SOURCES)))
PROGRAMSB=$(patsubst %.c,$(BIND)/%.bin,$(PROGRAMS))
KERNEL?=kernel.bin
BOOT?=boot.bin
.PHONY: all clean
all: $(BOOT) $(KERNEL) $(PROGRAMSB)
$(OBJECTS): ../.config

clean:
	@echo "  RM $(OBJECTS) $(KERNEL)"
	@rm -f $(OBJECTS) $(KERNEL)
	@$(MAKE) -C boot clean BIND=$(BIND) OBJD=$(OBJD) DEBUG=$(DEBUG) TARGET=$(BOOT)
	@rm -f $(BIND)/crt0.o $(BIND)/utils.o $(BIND)/libasm.o
	
$(KERNEL): $(OBJECTS)
	@echo "  LD $@"
	@mkdir -p $(dir $@)
	@$(LD) $(LDFLAGS) -T0x0000 -o $@ $(OBJECTS)
	
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

$(BIND)/utils.o: utils.c
	@echo "  CC $@"
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c $< -o $@

$(BIND)/libasm.o: x86/libasm.asm
	@echo "  AS $@"
	@mkdir -p $(dir $@)
	@$(AS) $(ASFLAGS) $< -o $@

$(BIND)/crt0.o: x86/crt0.asm
	@echo "  AS $@"
	@mkdir -p $(dir $@)
	@$(AS) $(ASFLAGS) $< -o $@

$(BIND)/%.bin: %.c $(BIND)/crt0.o $(BIND)/utils.o $(BIND)/libasm.o
	@echo "  CC $@"
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c $< -o $@.o
	@$(LD) $(LDFLAGS) -T0x0000 -o $@ $(BIND)/crt0.o $(BIND)/utils.o $(BIND)/libasm.o $@.o
	@rm -f $@.o
