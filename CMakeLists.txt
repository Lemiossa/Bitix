####################################
# CMakeLists.txt                   #
# Criado por Matheus Leme Da Silva #
####################################

cmake_minimum_required(VERSION 3.16)

set(ARCH "x86" CACHE STRING "Arquitetura do OS")

set_property(CACHE ARCH PROPERTY STRINGS "x86")

if(ARCH STREQUAL "x86")
	set(CMAKE_ASM_NASM_OBJECT_FORMAT "elf32")
else()
	message(FATAL_ERROR "Arquitetura não suportada: ${ARCH}")
endif()

project(Bitix C ASM_NASM)

set(KERNEL_FILE ${CMAKE_CURRENT_BINARY_DIR}/kernel/kernel.bin)
set(BOOTLDR_FILE ${CMAKE_CURRENT_BINARY_DIR}/bootldr/bootldr.bin)
set(IMAGE_FILE ${CMAKE_CURRENT_BINARY_DIR}/image/Bitix.img)
set(ROOT_DIR ${CMAKE_CURRENT_BINARY_DIR}/image/rootdir)
set(BOCHSRC_FILE ${CMAKE_CURRENT_BINARY_DIR}/bochsrc)
set(DISK_FILE ${CMAKE_CURRENT_BINARY_DIR}/disk.img)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(BASE_CFLAGS
	-std=c99 -Wall -Wextra -fno-PIC
	-fno-pie -fno-stack-protector
	-ffreestanding -fno-builtin
	-I${CMAKE_CURRENT_SOURCE_DIR}/include
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	add_compile_definitions(DEBUG)
endif()

add_subdirectory(libc)
add_subdirectory(bootldr)
add_subdirectory(kernel)

add_custom_command(
	OUTPUT ${ROOT_DIR}
	COMMAND mkdir -p ${ROOT_DIR}/system/boot
	COMMAND cp ${KERNEL_FILE} ${ROOT_DIR}/system/boot/kernel.sys
	DEPENDS ${KERNEL_FILE}
)

add_custom_command(
	OUTPUT ${IMAGE_FILE}
	COMMAND truncate -s 1440K ${IMAGE_FILE}
	COMMAND mkfs.fat -F 12 -R 64 -n "BITIX" ${IMAGE_FILE} > /dev/null 2>&1
	COMMAND dd if=${BOOTLDR_FILE} of=${IMAGE_FILE} bs=1 count=3 conv=notrunc > /dev/null 2>&1
	COMMAND dd if=${BOOTLDR_FILE}
			of=${IMAGE_FILE} bs=1 skip=60 seek=60 count=448 conv=notrunc > /dev/null 2>&1
	COMMAND dd if=${BOOTLDR_FILE} of=${IMAGE_FILE}
			bs=1 skip=512 seek=512 conv=notrunc > /dev/null 2>&1
	COMMAND mcopy -i ${IMAGE_FILE} ${ROOT_DIR}/* ::/ -s
	DEPENDS ${BOOTLDR_FILE} ${KERNEL_FILE} ${ROOT_DIR}
)

add_custom_target(image_file ALL
	DEPENDS bootldr-bin kernel-bin ${IMAGE_FILE}
)

add_custom_command(
	OUTPUT ${DISK_FILE}
	COMMAND cp ${IMAGE_FILE} ${DISK_FILE}
	DEPENDS image_file
)

add_custom_target(disk_file
	DEPENDS ${DISK_FILE}
)

add_custom_command(
	OUTPUT ${BOCHSRC_FILE}
	COMMAND printf \"clock: sync=delay, time0=local\\r\\n\" > ${BOCHSRC_FILE}
	COMMAND printf \"megs: 4\\r\\n\" >> ${BOCHSRC_FILE}
	COMMAND printf \"ata0: enabled=1, ioaddr1=0x1f0, ioaddr2=0x3f0, irq=14\\r\\n\" >> ${BOCHSRC_FILE}
	COMMAND printf \"ata0-master: type=disk, path=\\\"${DISK_FILE}\\\", mode=flat\\r\\n\" >> ${BOCHSRC_FILE}
	COMMAND printf \"ata0-slave: type=disk, path=\\\"${IMAGE_FILE}\\\", mode=flat\\r\\n\" >> ${BOCHSRC_FILE}
	COMMAND printf \"boot: disk\\r\\n\" >> ${BOCHSRC_FILE}
	COMMAND printf \"display_library: sdl2\\r\\n\" >> ${BOCHSRC_FILE}
	COMMAND printf \"cpu: ips=100000000, count=1\\r\\n\" >> ${BOCHSRC_FILE}
	DEPENDS disk_file
)

if(ARCH STREQUAL "x86")
	set(QEMU_FLAGS
		-audiodev alsa,id=audio0
		-machine pc,pcspk-audiodev=audio0
		-serial stdio
		-vga std
		-m 4M
		-smp 4
		-no-shutdown -no-reboot
	)

	add_custom_target(qemu
		COMMAND qemu-system-i386 ${QEMU_FLAGS}
				-drive file=${DISK_FILE},format=raw,if=ide,index=0
				-drive file=${IMAGE_FILE},format=raw,if=ide,index=1
		DEPENDS disk_file
	)

	add_custom_target(bochs
		COMMAND echo c | bochs -q -f ${BOCHSRC_FILE}
		DEPENDS ${BOCHSRC_FILE}
	)
endif()

