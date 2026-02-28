cmake_minimum_required(VERSION 3.16)

set(ARCH "x86" CACHE STRING "Arquitetura do OS")
set_property(CACHE ARCH PROPERTY STRINGS "x86")

if(ARCH STREQUAL "x86")
set(CMAKE_ASM_NASM_OBJECT_FORMAT "elf32")
endif()

project(Bitix C ASM_NASM)

set(KERNEL_FILE ${CMAKE_CURRENT_BINARY_DIR}/kernel.bin)
set(BOOTLDR_FILE ${CMAKE_CURRENT_BINARY_DIR}/bootldr.bin)
set(IMAGE_FILE ${CMAKE_CURRENT_BINARY_DIR}/Bitix.img)
set(ROOT_DIR ${CMAKE_CURRENT_BINARY_DIR}/rootdir)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(BASE_CFLAGS
	-std=c99 -Wall -Wextra -fno-PIC
	-fno-pie -fno-stack-protector
	-ffreestanding -fno-builtin
	-mno-mmx -mno-sse
	-I${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_subdirectory(libc)
add_subdirectory(bootldr)
add_subdirectory(kernel)

add_custom_command(
	OUTPUT ${ROOT_DIR}
	COMMAND mkdir -p ${ROOT_DIR}/system/boot
	COMMAND cp ${KERNEL_FILE} ${ROOT_DIR}/system/boot/kernel.sys
	DEPENDS kernel_bin ${KERNEL_FILE}
)

add_custom_target(root_dir
	DEPENDS ${ROOT_DIR}
)

add_custom_command(
	OUTPUT ${IMAGE_FILE}
	COMMAND truncate -s 1440K ${IMAGE_FILE}
	COMMAND mkfs.fat -F 12 -R 64 -n "BITIX" ${IMAGE_FILE}
	COMMAND dd if=bootldr.bin of=${IMAGE_FILE} bs=1 count=3 conv=notrunc
	COMMAND dd if=bootldr.bin
			of=${IMAGE_FILE} bs=1 skip=60 seek=60 count=448 conv=notrunc
	COMMAND dd if=bootldr.bin of=${IMAGE_FILE}
			bs=1 skip=512 seek=512 conv=notrunc
	COMMAND mcopy -i ${IMAGE_FILE} ${ROOT_DIR}/* ::/ -s
	DEPENDS bootldr_bin ${BOOTLDR_FILE} root_dir ${ROOT_DIR}
)

add_custom_target(image ALL
	DEPENDS ${IMAGE_FILE}
)

set(QEMU_FLAGS
	-audiodev alsa,id=audio0
	-machine pc,pcspk-audiodev=audio0
	-serial stdio
	-vga std
	-m 16
	-no-shutdown -no-reboot
)

if(ARCH STREQUAL "x86")
	add_custom_target(qemu
		COMMAND qemu-system-i386 ${QEMU_FLAGS} -fda ${IMAGE_FILE}
		DEPENDS image
	)
endif()

