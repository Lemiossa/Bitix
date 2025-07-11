#
# Makefile
# Created by Matheus Leme Da Silva
#
MAKEFLAGS+=--no-print-directory
BUILDV=0.1.0

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


-include src/.prog.files.mk

.PHONY: all clean run run-ng tools compile progs install menuconfig

all: $(IMAGE)

clean:
	@$(MAKE) -C tools/mkfs clean TARGET=../../$(FSTOOL)
	@$(MAKE) -C src clean BOOT=../../$(BOOT) KERNEL=../$(KERNEL) BIND=../$(BIN) OBJD=../$(OBJ) BUILDV=$(BUILDV)
	@rm -f $(IMAGE) $(FSTOOL)

run:
	@qemu-system-i386 $(QEMUFLAGS) -serial stdio

run-ng:
	@qemu-system-i386 $(QEMUFLAGS) -nographic

tools:
	@$(MAKE) -C tools/mkfs TARGET=../../$(FSTOOL)

compile:
	@find . -name "*.c" -o -name "*.h" | xargs clang-format -i
	@$(MAKE) -C src BOOT=../../$(BOOT) KERNEL=../$(KERNEL) BIND=../$(BIN) OBJD=../$(OBJ) BUILDV=$(BUILDV)

progs: src/.prog.files.mk
	@$(MAKE) -C src .prog.files.mk BOOT=../$(BOOT) KERNEL=../$(KERNEL) BIND=../$(BIN) OBJD=../$(OBJ) BUILDV=$(BUILDV)

$(IMAGE): tools compile progs
	@mkdir -p $(BIN) $(OBJ)
	@dd if=/dev/zero of=$(IMAGE) bs=1M count=1 status=none
	@./$(FSTOOL) $(BOOT) $(IMAGE) \
		/boot/bitixz:7=$(KERNEL) \
		/boot/boot.cfg:6=boot.cfg \
		$(PROGRAMS) \
		/home/README.md:6=README.md \
		/home/LICENSE:6=LICENSE

menuconfig:
	@python menuconfig.py
	@clear

install: $(IMAGE)
ifndef DEVICE
	$(error DEVICE not exists)
endif
	@dd if=$(IMAGE) of=$(DEVICE) bs=4M status=progress & sync
