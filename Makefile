####################################
# Makefile                         #
# Criado por Matheus Leme Da Silva #
####################################

BUILDDIR ?= $(CURDIR)/build
BINDIR ?= $(BUILDDIR)/bin
OBJDIR ?= $(BUILDDIR)/obj
DEPDIR ?= $(BUILDDIR)/dep
LIBDIR ?= $(BUILDDIR)/lib

ARCH ?= x86

IMAGE_FAT_SIZE := 12
IMAGE_SIZE := 1440K
IMAGE := $(BUILDDIR)/Bitix.img

BOOTLDR := $(BINDIR)/bootldr.bin
KERNEL := $(BINDIR)/kernel.bin
LIBC := $(LIBDIR)/libc.a

ROOTDIR := $(BUILDDIR)/rootdir

AR ?= ar
CC ?= gcc
NASM ?= nasm
OBJCOPY ?= objcopy

INCLUDES := $(CURDIR)/include

COMPILE_COMMANDS := $(CURDIR)/compile_commands.json
TMP_COMPILE_COMMANDS := $(CURDIR)/compile_commands.json.tmp

export BUILDDIR
export BINDIR
export OBJDIR
export DEPDIR
export LIBDIR
export ARCH
export INCLUDES

export LIBC

export AR
export CC
export NASM
export OBJCOPY

export TMP_COMPILE_COMMANDS

.PHONY: all
all: prepare_cpl_cmds image finish_cpl_cmds

.PHONY: clean
clean:
	@$(MAKE) -C bootldr TARGET=$(BOOTLDR) clean
	@$(MAKE) -C kernel TARGET=$(KERNEL) clean
	@$(MAKE) -C libc TARGET=$(LIBC) clean
	@rm -f $(IMAGE)

.PHONY: prepare_cpl_cmds
prepare_cpl_cmds:
	@echo "[" > $(TMP_COMPILE_COMMANDS)

.PHONY: finish_cpl_cmds
finish_cpl_cmds:
	@sed -i '$$d' $(TMP_COMPILE_COMMANDS)
	@echo "    }" >> $(TMP_COMPILE_COMMANDS)
	@echo "]" >> $(TMP_COMPILE_COMMANDS)
	@if [ -f $(COMPILE_COMMANDS) ] && cmp -s $(TMP_COMPILE_COMMANDS) $(COMPILE_COMMANDS); then \
		rm $(TMP_COMPILE_COMMANDS); \
	else \
		mv $(TMP_COMPILE_COMMANDS) $(COMPILE_COMMANDS); \
	fi

.PHONY: bootldr
bootldr:
	@$(MAKE) -C bootldr TARGET=$(BOOTLDR)

.PHONY: kernel
kernel:
	@$(MAKE) -C kernel TARGET=$(KERNEL)

.PHONY: libc
libc:
	@$(MAKE) -C libc TARGET=$(LIBC)

.PHONY: rootdir
rootdir:
	@mkdir -p $(ROOTDIR)/sistema/boot/
	@cp $(KERNEL) $(ROOTDIR)/sistema/boot/kernel.sys

.PHONY: image
image: libc bootldr kernel rootdir
	@echo "  GERANDO IMAGEM $(IMAGE)"
	@dd if=/dev/zero of=$(IMAGE) bs=1 count=0 seek=$(IMAGE_SIZE) >/dev/null 2>&1
	@mkfs.fat -F $(IMAGE_FAT_SIZE) -n "BITIX" -R 64 $(IMAGE) >/dev/null 2>&1
	@dd if=$(BOOTLDR) of=$(IMAGE) bs=1 count=3 conv=notrunc >/dev/null 2>&1
	@dd if=$(BOOTLDR) of=$(IMAGE) bs=1 skip=60 seek=60 count=448 conv=notrunc >/dev/null 2>&1
	@dd if=$(BOOTLDR) of=$(IMAGE) bs=1 skip=512 seek=512 conv=notrunc >/dev/null 2>&1
	@mcopy -i $(IMAGE) $(ROOTDIR)/* "::/" -s >/dev/null 2>&1

ifeq ($(ARCH),x86)
QEMU := qemu-system-i386
QEMUFLAGS := -drive file=$(IMAGE),format=raw,if=ide,media=disk \
			 -audiodev alsa,id=audio0 \
			 -machine pc,pcspk-audiodev=audio0 \
			 -serial stdio -m 4M -vga std \
			 -smp 1 -enable-kvm

BOCHS := bochs
else
$(error Arquitetura não suportada: $(ARCH))
endif

.PHONY: qemu
qemu: image
	@echo "  QEMU        $(QEMUFLAGS)"
	@$(QEMU) $(QEMUFLAGS)

.PHONY: bochs
bochs: image
	@echo "  BOCHS"
	@printf "clock: sync=delay, time0=local\r\n" > $(BUILDDIR)/bochsrc
	@printf "megs: 4\r\n" >> $(BUILDDIR)/bochsrc
	@printf "ata0: enabled=1, ioaddr1=0x1f0, ioaddr2=0x3f0, irq=14\r\n" >> $(BUILDDIR)/bochsrc
	@printf "ata0-master: type=disk, path=\"$(IMAGE)\", mode=flat\r\n" >> $(BUILDDIR)/bochsrc
	@printf "boot: disk\r\n" >> $(BUILDDIR)/bochsrc
	@printf "display_library: sdl2\r\n" >> $(BUILDDIR)/bochsrc
	@printf "cpu: ips=100000000, count=1\r\n" >> $(BUILDDIR)/bochsrc
	@echo c | $(BOCHS) -q -f $(BUILDDIR)/bochsrc
