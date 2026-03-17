####################################
# Makefile                         #
# Criado por Matheus Leme Da Silva #
####################################

# BUILDDIR é passado pelo makefile principal
BUILDDIR ?= $(CURDIR)/build
BINDIR := $(BUILDDIR)/bootldr/bin
OBJDIR := $(BUILDDIR)/bootldr/obj
DEPDIR := $(BUILDDIR)/bootldr/dep
LIBDIR := $(BUILDDIR)/bootldr/lib

ARCH ?= x86

CC ?= gcc
NASM ?= nasm
AR ?= ar

COMPILE_COMMANDS ?= $(CURDIR)/compile_commands.json

INCLUDES ?=

INCLUDES += $(CURDIR)/source

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
	$(addprefix -I,$(INCLUDES))

ifeq ($(ARCH),x86)
	CFLAGS += -m32 -march=i386
	NASMFLAGS := -f elf32
else
	$(error Arquitetura não suportada: $(ARCH))
endif

TARGET ?= $(BINDIR)/bootldr.bin

SOURCE := \
		  string.c \
		  stdio.c \
		  stdlib.c

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

$(TARGET): $(OBJ)
	@mkdir -p $(dir $@)
	@echo "  AR          $@"
	@$(AR) rcs $@ $^

$(OBJDIR)/c/%.o: $(CURDIR)/source/%.c
	@echo "  CC          $@"
	@mkdir -p $(dir $@) $(dir $(DEPDIR)/c/$*)
	@$(CC) $(CFLAGS) -c $< -o $@ -MMD -MF $(DEPDIR)/c/$*.d
	@echo "    {" >> $(COMPILE_COMMANDS)
	@echo "        \"directory\": \"$(CURDIR)\"," >> $(COMPILE_COMMANDS)
	@echo "        \"arguments\": [" >> $(COMPILE_COMMANDS)
	@echo "            \"$(CC)\"" >> $(COMPILE_COMMANDS)
	@for flag in $(CFLAGS) -c "$<" -o "$@"; do \
		printf ",\n            \"$$flag\"" >> $(COMPILE_COMMANDS); \
		done
	@echo "" >> $(COMPILE_COMMANDS)
	@echo "        ]," >> $(COMPILE_COMMANDS)
	@echo "        \"file\": \"$<\"" >> $(COMPILE_COMMANDS)
	@echo "    }," >> $(COMPILE_COMMANDS)

$(OBJDIR)/asm/%.o: $(CURDIR)/source/%.asm
	@echo "  NASM        $@"
	@mkdir -p $(dir $@) $(dir $(DEPDIR)/asm/$*)
	@$(NASM) $(NASMFLAGS) $< -o $@ -MD $(DEPDIR)/asm/$*.d

-include $(DEP)

