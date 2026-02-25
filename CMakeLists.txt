cmake_minimum_required(VERSION 3.16)

set(ARCH "x86" CACHE STRING "Arquitetura do OS")
set_property(CACHE ARCH PROPERTY STRINGS "x86")

if(ARCH STREQUAL "x86")
set(CMAKE_ASM_NASM_OBJECT_FORMAT "elf32")
endif()

project(Bitix C ASM_NASM)

set(KERNEL_FILE
	${CMAKE_CURRENT_BINARY_DIR}/kernel.bin
)

set(BOOTLDR_FILE
	${CMAKE_CURRENT_BINARY_DIR}/bootldr.bin
)

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

add_custom_target(root
	COMMAND mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/root/system/boot
	COMMAND cp ${KERNEL_FILE} root/system/boot/kernel.sys
	DEPENDS kernel_bin
)

add_custom_command(
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/Bitix.img
	COMMAND truncate -s 1440K ${CMAKE_CURRENT_BINARY_DIR}/Bitix.img
	COMMAND mkfs.fat -F 12 -R 64 -n "BITIX" ${CMAKE_CURRENT_BINARY_DIR}/Bitix.img
	COMMAND dd if=bootldr.bin of=${CMAKE_CURRENT_BINARY_DIR}/Bitix.img bs=1 count=3 conv=notrunc
	COMMAND dd if=bootldr.bin
			of=${CMAKE_CURRENT_BINARY_DIR}/Bitix.img bs=1 skip=60 seek=60 count=448 conv=notrunc
	COMMAND dd if=bootldr.bin of=$@ bs=1 skip=512 seek=512 conv=notrunc
	COMMAND mcopy -i ${CMAKE_CURRENT_BINARY_DIR}/Bitix.img root/* ::/ -s
	DEPENDS bootldr_bin root
)

add_custom_target(image ALL
	DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/Bitix.img
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
		COMMAND qemu-system-i386 ${QEMU_FLAGS} -fda ${CMAKE_CURRENT_BINARY_DIR}/Bitix.img
		DEPENDS image
	)
endif()

