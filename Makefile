#
# Makefile
# Created by Matheus Leme Da Silva
#

# Build configs
VERSION = v0.1.0
BUILD_DATE = $(shell date +%Y-%m-%d)

# Tool Settings
MKFS = $(PWD)/fstools/mkfs
LS = $(PWD)/fstools/ls

CC = gcc
CFLAGS = -Wall -Wextra -Werror -O2 -I ./fstools

# Image and fs configs
IMAGE = bitix-$(VERSION)-i386-$(BUILD_DATE).img
BS = 1M
SIZE = 16
BLOCK_SIZE = 4096 # 4 KiB blocks

# Others
BUILDDIR = $(PWD)/build
BINDIR = $(BUILDDIR)/bin
IMAGEDIR = $(BUILDDIR)/images

BOOTLOADER = $(BINDIR)/bootloader.bin
KERNEL = $(BINDIR)/kernel.bin

# Qemu configs
QEMU = qemu-system-i386
QEMUFLAGS = -m 32M \
						-M pc \
						-drive file=$(IMAGEDIR)/$(IMAGE),format=raw,if=ide \
						-enable-kvm # Remove if your PC doesn't support it

QEMUFLAGS += \
						-smp 2 \
						-rtc base=utc \
						-serial stdio \
						-monitor vc \
						-device isa-debug-exit,iobase=0xf4,iosize=0x04 \
						-audiodev alsa,id=audio0,out.frequency=44100,out.channels=2 \
						-device sb16,audiodev=audio0 \
						-net nic,model=ne2k_isa \
						-net user \
						-vga std \

.PHONY: all clean qemu always install
all: $(IMAGE)

clean: 
	rm -rf $(MKFS) $(LS)
	rm -rf $(BUILDDIR)
	$(MAKE) -C boot clean TARGET=$(BOOTLOADER) BUILDV=$(VERSION)
	$(MAKE) -C kernel clean TARGET=$(KERNEL) BUILDV=$(VERSION)

qemu:
	$(LS) $(IMAGEDIR)/$(IMAGE) /boot
	$(QEMU) $(QEMUFLAGS)

debug:
	$(LS) $(IMAGEDIR)/$(IMAGE) /boot
	$(QEMU) $(QEMUFLAGS) -s -S & \
	sleep 1 && \
	gdb -ex "target remote localhost:1234" \
			-ex "layout regs"

always:
	mkdir -p $(IMAGEDIR) $(BUILDDIR) $(BINDIR)

$(IMAGE): always $(MKFS) $(LS) $(BOOTLOADER) $(KERNEL)
	dd if=/dev/zero of=$(IMAGEDIR)/$(IMAGE) bs=$(BS) count=$(SIZE)
	$(MKFS) $(BOOTLOADER) $(IMAGEDIR)/$(IMAGE) \
		/boot/bitixz:0755=$(KERNEL) \
		--block-size=$(BLOCK_SIZE)

$(MKFS): $(MKFS).c
	$(CC) $(CFLAGS) -o $(MKFS) $(MKFS).c

$(LS): $(LS).c
	$(CC) $(CFLAGS) -o $(LS) $(LS).c

$(KERNEL): always
	$(MAKE) -C kernel TARGET=$(KERNEL) BUILDV=$(VERSION)

$(BOOTLOADER): always
	$(MAKE) -C boot TARGET=$(BOOTLOADER) BUILDV=$(VERSION)

install: $(IMAGE)
	@echo "Available disk devices (excluding /dev/sda):"
	@lsblk -d -n -o NAME,SIZE,MODEL | grep -v '^sda' | awk '{print "/dev/" $$1, $$2, $$3}'
	@echo
	@read -p "Enter the target device to install (e.g. /dev/sdb): " device; \
	if [ "$$device" = "" ]; then \
		echo "No device entered. Installation cancelled."; \
		exit 1; \
	fi; \
	if [ "$$device" = "/dev/sda" ]; then \
		echo "Refusing to write to /dev/sda for safety reasons. Installation cancelled."; \
		exit 1; \
	fi; \
	if [ ! -b "$$device" ]; then \
		echo "Device $$device not found or is not a block device. Installation cancelled."; \
		exit 1; \
	fi; \
	echo "== WARNING: This will WRITE the image directly to $$device =="; \
	echo "== If you choose the wrong device, you WILL LOSE DATA! =="; \
	read -p "Are you sure you want to continue? (yes/no): " ans; \
	if [ "$$ans" = "yes" ]; then \
		echo "Writing image to $$device..."; \
		sudo dd if=$(IMAGEDIR)/$(IMAGE) of=$$device bs=4M status=progress conv=fsync; \
		echo "Syncing disk..."; \
		sync; \
		echo "Done! Please reboot or set up your bootloader accordingly."; \
	else \
		echo "Installation cancelled."; \
	fi
