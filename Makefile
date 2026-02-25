####################################
# Makefile                         #
# Criado por Matheus Leme Da Silva #
####################################

# x86 ou x86-64
ARCH := x86
IMAGE := Bitix.img
FAT_TYPE := 12
IMAGE_SIZE := 1440K
DEBUG ?= false

ifneq ($(ARCH),x86)
	$(error Arquitetura não suportada: $(ARCH))
endif

BUILDDIR := $(CURDIR)/build
BINDIR := $(BUILDDIR)/bin
OBJDIR := $(BUILDDIR)/obj
DEPDIR := $(BUILDDIR)/dep
LIBDIR := $(BUILDDIR)/lib
IMAGESDIR := $(BUILDDIR)/images
ROOTDIR := $(BUILDDIR)/rootdir

INCLUDES := $(CURDIR)/include

CC := gcc
LD := ld
AR := ar

INCLUDES := $(addprefix -I,$(INCLUDES))

export BUILDDIR
export BINDIR
export OBJDIR
export DEPDIR
export LIBDIR
export DEBUG
export INCLUDES
export CC
export LD
export AR
export ARCH

BOOTLOADER := $(BINDIR)/bootldr.bin
KERNEL := $(BINDIR)/kernel.bin
LIBC := $(LIBDIR)/libc.a

DISK ?= $(CURDIR)/disk.img

IMAGE := $(addprefix $(IMAGESDIR)/,$(IMAGE))

ifeq ($(ARCH),x86)
	QEMU := qemu-system-i386
	QEMUFLAGS := \
				 -audiodev alsa,id=audio0 \
				 -machine pc,pcspk-audiodev=audio0 \
				 -serial stdio -vga std -m 16 \
				 -no-shutdown -no-reboot
endif

.PHONY: all
all: $(IMAGE)

.PHONY: clean
clean:
	$(MAKE) -C bootldr TARGET=$(BOOTLOADER) clean
	$(MAKE) -C kernel TARGET=$(KERNEL) clean
	$(MAKE) -C libc TARGET=$(LIBC) clean
	rm -f $(IMAGE)

.PHONY: qemu
qemu: install
	$(QEMU) $(QEMUFLAGS) -fda $(DISK)

.PHONY: install
install: $(IMAGE)
	dd if=$< of=$(DISK) bs=4M status=progress && sync

.PHONY: bootloader
bootloader:
	$(MAKE) -C bootldr TARGET=$(BOOTLOADER)

.PHONY: kernel
kernel:
	$(MAKE) -C kernel TARGET=$(KERNEL)

.PHONY: libc
libc:
	$(MAKE) -C libc TARGET=$(LIBC)

$(ROOTDIR): $(KERNEL)
	mkdir -p $@ $@/system $@/system/boot
	cp $(KERNEL) $@/system/boot/kernel.sys

$(IMAGE): libc bootloader kernel $(ROOTDIR)
	mkdir -p $(dir $@)
	truncate -s $(IMAGE_SIZE) $@
	mkfs.fat -F $(FAT_TYPE) -R 64 -n "BITIX" $@
	dd if=$(BOOTLOADER) of=$@ bs=1 count=3 conv=notrunc
ifeq ($(FAT_TYPE),32)
	dd if=$(BOOTLOADER) of=$@ bs=1 skip=90 seek=90 count=420 conv=notrunc
else
	dd if=$(BOOTLOADER) of=$@ bs=1 skip=62 seek=62 count=448 conv=notrunc
endif
	dd if=$(BOOTLOADER) of=$@ bs=1 skip=512 seek=512 conv=notrunc
	mcopy -i $@ $(ROOTDIR)/* ::/ -s

