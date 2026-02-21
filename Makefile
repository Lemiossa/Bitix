####################################
# Makefile                         #
# Criado por Matheus Leme Da Silva #
####################################

# x86 ou x86-64
ARCH := x86
IMAGE := Bitix.img

ifneq ($(ARCH),x86)
	$(error Arquitetura não suportada: $(ARCH))
endif

BUILDDIR := $(CURDIR)/build
BINDIR := $(BUILDDIR)/bin
OBJDIR := $(BUILDDIR)/obj
DEPDIR := $(BUILDDIR)/dep
LIBDIR := $(BUILDDIR)/lib
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
export INCLUDES
export CC
export LD
export AR
export ARCH

BOOTLOADER := $(BINDIR)/bootldr.bin
KERNEL := $(BINDIR)/kernel.bin
LIBC := $(LIBDIR)/libc.a

ifeq ($(ARCH),x86)
	QEMU := qemu-system-i386
	QEMUFLAGS := \
				 -audiodev alsa,id=audio0 \
				 -machine q35,pcspk-audiodev=audio0 \
				 -serial stdio -vga std \
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
qemu: $(IMAGE)
	$(QEMU) $(QEMUFLAGS) -fda $(IMAGE)

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
	dd if=/dev/zero of=$@ bs=1440K count=1
	mkfs.fat -F 12 -R 64 -n "BITIX" $@
	dd if=$(BOOTLOADER) of=$@ bs=1 count=3 conv=notrunc
	dd if=$(BOOTLOADER) of=$@ bs=1 skip=62 seek=62 count=448 conv=notrunc
	dd if=$(BOOTLOADER) of=$@ bs=1 skip=512 seek=512 conv=notrunc
	mcopy -i $@ $(ROOTDIR)/* ::/ -s

