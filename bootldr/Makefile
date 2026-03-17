####################################
# Makefile                         #
# Criado por Matheus Leme Da Silva #
####################################

# BUILDDIR é passado pelo makefile principal
BUILDDIR ?= $(CURDIR)/build
BUILDDIR := $(addsuffix /bootldr,$(BUILDDIR))
BINDIR := $(BUILDDIR)/bin
OBJDIR := $(BUILDDIR)/obj
DEPDIR := $(BUILDDIR)/dep
LIBDIR := $(BUILDDIR)/lib

ARCH ?= x86

CC ?= gcc
NASM ?= nasm
OBJCOPY ?= objcopy

TMP_COMPILE_COMMANDS ?= $(CURDIR)/compile_commands.json

LIBC ?= $(LIBDIR)/libc.a

LINKER_FILE := $(CURDIR)/source/linker.ld

INCLUDES ?=

INCLUDES += $(CURDIR)/source
LIBS := $(LIBC)

CFLAGS := \
	-std=c99 \
	-Wall \
	-Wextra \
	-fno-PIC \
	-fno-pie \
	-fno-stack-protector \
	-ffreestanding \
	-fno-builtin \
	-ffunction-sections -fdata-sections \
	-m32 -march=i386 \
	$(addprefix -I,$(INCLUDES))

TARGET ?= $(BINDIR)/bootldr.bin

SOURCE := \
	entry.asm \
	main.c \
	disk.c \
	vesa.c \
	e820.c \
	file.c \
	util.c \
	vga.c \
	fat.c

SOURCE := $(addprefix $(CURDIR)/source/,$(SOURCE))

SOURCE_ASM := $(filter %.asm,$(SOURCE))
SOURCE_C := $(filter %.c,$(SOURCE))

OBJ := $(patsubst $(CURDIR)/source/%.asm,$(OBJDIR)/asm/%.o,$(SOURCE_ASM)) \
	   $(patsubst $(CURDIR)/source/%.c,$(OBJDIR)/c/%.o,$(SOURCE_C))

DEP := $(patsubst $(CURDIR)/source/%.asm,$(DEPDIR)/asm/%.d,$(SOURCE_ASM)) \
	   $(patsubst $(CURDIR)/source/%.c,$(DEPDIR)/c/%.d,$(SOURCE_C))

.PHONY: all
all: $(TARGET)

.PHONY: clean
clean:
	@echo "  RM          $(TARGET) $(OBJ) $(DEP)"
	@rm -f $(TARGET) $(OBJ) $(DEP)

$(BINDIR)/stage1.bin: $(CURDIR)/source/bootsect.asm
	@mkdir -p $(dir $@)
	@echo "  NASM        $@"
	@$(NASM) -f bin $< -o $@

$(BINDIR)/stage2.elf: $(OBJ) $(LIBS)
	@mkdir -p $(dir $@)
	@echo "  LINK        $@"
	@$(CC) -m32 -nostdlib -static \
		-T$(LINKER_FILE) \
		-Wl,-melf_i386 \
		-Wl,--gc-sections \
		-o $@ $^

$(BINDIR)/stage2.bin: $(BINDIR)/stage2.elf
	@mkdir -p $(dir $@)
	@echo "  OBJCOPY     $@"
	@$(OBJCOPY) -O binary $< $@

$(TARGET): $(BINDIR)/stage1.bin $(BINDIR)/stage2.bin
	@mkdir -p $(dir $@)
	@cat $^ > $@

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
	@$(NASM) -f elf32 $< -o $@ -MD $(DEPDIR)/asm/$*.d

-include $(DEP)
