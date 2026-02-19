####################################
# Makefile                         #
# Criado por Matheus Leme Da Silva #
####################################

# x86 ou x86-64
ARCH := x86
IMAGE := Bitix.img

ifneq ($(ARCH),x86)
	$(error Unsuported arch: $(ARCH))
endif

BUILDDIR := $(CURDIR)/Build
BINDIR := $(BUILDDIR)/Bin
OBJDIR := $(BUILDDIR)/Obj
DEPDIR := $(BUILDDIR)/Dep

CC := clang
LD := ld.lld

export BUILDDIR
export BINDIR
export OBJDIR
export DEPDIR
export CC
export LD

BOOTLOADER := $(BINDIR)/Bootldr.bin

ifeq ($(ARCH),x86)
	QEMU := qemu-system-i386
	QEMUFLAGS := \
				 -audiodev alsa,id=audio0 \
				 -machine pc,pcspk-audiodev=audio0 \
				 -serial stdio -vga std
endif


.PHONY: all
all: $(IMAGE)

.PHONY: clean
clean:
	$(MAKE) -C Bootldr TARGET=$(BOOTLOADER) clean
	rm -f $(IMAGE)

.PHONY: qemu
qemu: $(IMAGE)
	$(QEMU) $(QEMUFLAGS) -fda $(IMAGE)

.PHONY: bootloader
bootloader:
	$(MAKE) -C Bootldr TARGET=$(BOOTLOADER)

$(IMAGE): bootloader
	dd if=/dev/zero of=$@ bs=1440K count=1
	mkfs.fat -F 12 -R 64 -n "BITIX" $@
	dd if=$(BOOTLOADER) of=$@ bs=1 count=3 conv=notrunc
	dd if=$(BOOTLOADER) of=$@ bs=1 skip=62 seek=62 count=448 conv=notrunc
	dd if=$(BOOTLOADER) of=$@ bs=1 skip=512 seek=512 conv=notrunc


