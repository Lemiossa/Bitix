####################################
# Makefile                         #
# Criado por Matheus Leme Da Silva #
####################################

# BUILDDIR é passado pelo makefile principal
BUILDDIR ?= $(CURDIR)/build
BUILDDIR := $(addsuffix /kernel,$(BUILDDIR))
BINDIR := $(BUILDDIR)/bin
OBJDIR := $(BUILDDIR)/obj
DEPDIR := $(BUILDDIR)/dep
LIBDIR := $(BUILDDIR)/lib

ARCH ?= x86

CC ?= gcc
NASM ?= nasm

LIBC ?= $(LIBDIR)/libc.a

LINKER_FILE := $(CURDIR)/source/arch/$(ARCH)/linker.ld

INCLUDES ?=
INCLUDES += $(CURDIR)/include \
			$(CURDIR)/include/arch/$(ARCH)
LIBS := $(LIBC)

TMP_COMPILE_COMMANDS ?= $(CURDIR)/compile_commands.json

CFLAGS := \
	-std=c99 -O3 \
	-Wall \
	-Wextra \
	-fno-PIC \
	-fno-pie \
	-fno-stack-protector \
	-ffreestanding \
	-fno-builtin \
	-ffunction-sections -fdata-sections \
	$(addprefix -I,$(INCLUDES))

ifeq ($(ARCH),x86)
	CFLAGS += -m32 -march=i386
	NASMFLAGS := -f elf32
	LDFLAGS := \
			   -m32 -nostdlib -static \
			   -T$(LINKER_FILE) \
			   -Wl,-melf_i386 \
			   -Wl,--gc-sections
else
	$(error Arquitetura não suportada: $(ARCH))
endif

TARGET ?= $(BINDIR)/bootldr.bin

SOURCE := \
		  arch/$(ARCH)/entry.asm \
		  arch/$(ARCH)/intr.asm \
		  arch/$(ARCH)/cpuid.asm \
		  arch/$(ARCH)/cpuid.c \
		  arch/$(ARCH)/acpi.c \
		  arch/$(ARCH)/gdt.c \
		  arch/$(ARCH)/idt.c \
		  arch/$(ARCH)/fpu.c \
		  arch/$(ARCH)/pmm.c \
		  arch/$(ARCH)/vmm.c \
		  graphics.c \
		  terminal.c \
		  debug.c \
		  sched.c \
		  panic.c \
		  heap.c \
		  main.c \
		  util.c \
		  pic.c \
		  pit.c \
		  pci.c \
		  ata.c \
		  fat.c \
		  vga.c \
		  vfs.c

SOURCE := $(addprefix $(CURDIR)/source/,$(SOURCE))

SOURCE_ASM := $(filter %.asm,$(SOURCE))
SOURCE_C := $(filter %.c,$(SOURCE))

OBJ := $(patsubst $(CURDIR)/source/%.asm,$(OBJDIR)/asm/%.o,$(SOURCE_ASM)) \
	   $(patsubst $(CURDIR)/source/%.c,$(OBJDIR)/c/%.o,$(SOURCE_C))

DEP := $(patsubst $(CURDIR)/source/%.asm,$(DEPDIR)/asm/%.d,$(SOURCE_ASM)) \
	   $(patsubst $(CURDIR)/source/%.c,$(DEPDIR)/c/%.d,$(SOURCE_C))

.PHONY: all
all: fonts $(TARGET)

.PHONY: clean
clean:
	@echo "  RM          $(TARGET) $(OBJ) $(DEP)"
	@rm -f $(TARGET) $(OBJ) $(DEP) $(CURDIR)/include/font8x8.h

.PHONY: fonts
fonts: $(CURDIR)/include/font8x8.h

$(BINDIR)/kernel.elf: $(OBJ) $(LIBS)
	@mkdir -p $(dir $@)
	@echo "  LINK        $@"
	@$(CC) $(LDFLAGS) -o $@ $^

$(TARGET): $(BINDIR)/kernel.elf
	@mkdir -p $(dir $@)
	@echo "  OBJCOPY     $@"
	@$(OBJCOPY) -O binary $< $@

$(OBJDIR)/c/%.o: $(CURDIR)/source/%.c
	@echo "  CC          $@"
	@mkdir -p $(dir $@) $(dir $(DEPDIR)/c/$*)
	@$(CC) $(CFLAGS) -c $< -o $@ -MMD -MF $(DEPDIR)/c/$*.d
	@echo "    {" >> $(TMP_COMPILE_COMMANDS)
	@echo "        \"directory\": \"$(CURDIR)\"," >> $(TMP_COMPILE_COMMANDS)
	@echo "        \"command\": \"$(CC) $(CFLAGS) -c $< -o $@\"," >> $(TMP_COMPILE_COMMANDS)
	@echo "        \"file\": \"$<\"," >> $(TMP_COMPILE_COMMANDS)
	@echo "        \"output\": \"$@\"" >> $(TMP_COMPILE_COMMANDS)
	@echo "    }," >> $(TMP_COMPILE_COMMANDS)

$(OBJDIR)/asm/%.o: $(CURDIR)/source/%.asm
	@echo "  NASM        $@"
	@mkdir -p $(dir $@) $(dir $(DEPDIR)/asm/$*)
	@$(NASM) $(NASMFLAGS) $< -o $@ -MD $(DEPDIR)/asm/$*.d

$(CURDIR)/include/font8x8.h: $(CURDIR)/font8x8.txt
	@echo "  GENERANDO FONTE 8x8..."
	@python3 font2c.py $< $@

-include $(DEP)
