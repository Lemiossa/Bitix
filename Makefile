#
# Makefile
# Created by Matheus Leme Da Silva
#
MAKEFLAGS+=--no-print-directory
BUILDV=0.0.2

# Dirs
BUILD=build
BIN=$(BUILD)/binaries
OBJ=$(BUILD)/objects

# Files
FSTOOL=mkfs.bfx
KERNEL=$(BIN)/kernel.bin
BOOT=$(BIN)/boot.bin

IMAGE=$(BUILD)/Bitix.img

KERNELIMAGE=kernel.bin
QEMUFLAGS=-m 2048\
					-drive file=$(IMAGE),format=raw \
					-enable-kvm \
					-smp 2 \
					-rtc base=utc \
					-monitor vc \
					-net nic,model=ne2k_isa -net user

ifeq (debug,true)
	QEMUFLAGS+=-d int
endif

.PHONY: all clean run run-ng tools compile install menuconfig

all: $(IMAGE)

clean:
	@make -C tools clean TARGET=../$(FSTOOL)
	@make -C src clean BOOT=../../$(BOOT) KERNEL=../$(KERNEL) BIND=../$(BIN) OBJD=../$(OBJ) BUILDV=$(BUILDV)
	@rm -f $(IMAGE) $(FSTOOL)

run: $(IMAGE)
	@qemu-system-i386 $(QEMUFLAGS) -serial stdio

run-ng: $(IMAGE)
	@qemu-system-i386 $(QEMUFLAGS) -nographic

tools:
	@make -C tools TARGET=../$(FSTOOL)

compile:
	@make -C src BOOT=../../$(BOOT) KERNEL=../$(KERNEL) BIND=../$(BIN) OBJD=../$(OBJ) BUILDV=$(BUILDV)

$(IMAGE): tools compile
	@mkdir -p $(BIN) $(OBJ)
	@dd if=/dev/zero of=$(IMAGE) bs=1M count=2 status=none
	@./$(FSTOOL) $(BOOT) $(IMAGE) /boot/bitixz:7=$(KERNEL) /boot/boot.cfg:6=boot.cfg

menuconfig:
	@python menuconfig.py
	@clear

install: $(IMAGE)
ifndef DEVICE
	$(error DEVICE not exists)
endif
	@dd if=$(IMAGE) of=$(DEVICE) bs=4M status=progress & sync
