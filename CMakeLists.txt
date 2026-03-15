####################################
# CMakeLists.txt                   #
# Criado por Matheus Leme Da Silva #
####################################

CMAKE_MINIMUM_REQUIRED(VERSION 3.16)

SET(ARCH
	"x86"
	CACHE STRING "Arquitetura do OS")

SET_PROPERTY(CACHE ARCH PROPERTY STRINGS "x86")

IF(ARCH STREQUAL "x86")
	SET(CMAKE_ASM_NASM_OBJECT_FORMAT "elf32")
ELSE()
	MESSAGE(FATAL_ERROR "Arquitetura não suportada: ${ARCH}")
ENDIF()

PROJECT(Bitix C ASM_NASM)

SET(KERNEL_FILE ${CMAKE_CURRENT_BINARY_DIR}/kernel/kernel.bin)
SET(KERNEL_MAP_FILE ${CMAKE_CURRENT_BINARY_DIR}/kernel/kernel.map)
SET(BOOTLDR_FILE ${CMAKE_CURRENT_BINARY_DIR}/bootldr/bootldr.bin)
SET(IMAGE_FILE ${CMAKE_CURRENT_BINARY_DIR}/image/Bitix.img)
SET(ROOT_DIR ${CMAKE_CURRENT_BINARY_DIR}/image/rootdir)
SET(BOCHSRC_FILE ${CMAKE_CURRENT_BINARY_DIR}/bochsrc)

SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)

SET(BASE_CFLAGS
	-std=c99
	-Wall
	-Wextra
	-fno-PIC
	-fno-pie
	-fno-stack-protector
	-ffreestanding
	-fno-builtin
	-I${CMAKE_CURRENT_SOURCE_DIR}/include)

IF(CMAKE_BUILD_TYPE STREQUAL "Debug")
	ADD_COMPILE_DEFINITIONS(DEBUG)
ENDIF()

ADD_SUBDIRECTORY(libc)
ADD_SUBDIRECTORY(bootldr)
ADD_SUBDIRECTORY(kernel)

ADD_CUSTOM_COMMAND(
	OUTPUT ${ROOT_DIR}
	COMMAND mkdir -p ${ROOT_DIR}/sistema/boot ${ROOT_DIR}/sistema/binarios
	COMMAND cp ${KERNEL_FILE} ${ROOT_DIR}/sistema/boot/kernel.sys
	COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/texto.txt ${ROOT_DIR}/texto.txt
	DEPENDS ${KERNEL_FILE} texto.txt
)

ADD_CUSTOM_COMMAND(
	OUTPUT ${IMAGE_FILE}
	COMMAND truncate -s 1440K ${IMAGE_FILE}
	COMMAND mkfs.fat -F 12 -R 64 -n "BITIX" ${IMAGE_FILE} > /dev/null 2>&1
	COMMAND dd if=${BOOTLDR_FILE} of=${IMAGE_FILE} bs=1 count=3 conv=notrunc >
			/dev/null 2>&1
	COMMAND dd if=${BOOTLDR_FILE} of=${IMAGE_FILE} bs=1 skip=60 seek=60
			count=448 conv=notrunc > /dev/null 2>&1
	COMMAND dd if=${BOOTLDR_FILE} of=${IMAGE_FILE} bs=1 skip=512 seek=512
			conv=notrunc > /dev/null 2>&1
	COMMAND mcopy -i ${IMAGE_FILE} ${ROOT_DIR}/* ::/ -s
	DEPENDS ${BOOTLDR_FILE} ${KERNEL_FILE} ${ROOT_DIR}
)

ADD_CUSTOM_TARGET(image_file ALL DEPENDS bootldr-bin kernel-bin ${IMAGE_FILE})

ADD_CUSTOM_COMMAND(
	OUTPUT ${BOCHSRC_FILE}
	COMMAND printf \"clock: sync=delay, time0=local\\r\\n\" > ${BOCHSRC_FILE}
	COMMAND printf \"megs: 4\\r\\n\" >> ${BOCHSRC_FILE}
	COMMAND printf \"ata0: enabled=1, ioaddr1=0x1f0, ioaddr2=0x3f0,
			irq=14\\r\\n\" >> ${BOCHSRC_FILE}
	COMMAND printf \"ata0-master: type=disk, path=\\\"${IMAGE_FILE}\\\",
			mode=flat\\r\\n\" >> ${BOCHSRC_FILE}
	COMMAND printf \"boot: disk\\r\\n\" >> ${BOCHSRC_FILE}
	COMMAND printf \"display_library: sdl2\\r\\n\" >> ${BOCHSRC_FILE}
	COMMAND printf \"cpu: ips=100000000, count=1\\r\\n\" >> ${BOCHSRC_FILE}
	DEPENDS image_file
)

IF(ARCH STREQUAL "x86")
	SET(QEMU_FLAGS
		-audiodev alsa,id=audio0
		-machine pc,pcspk-audiodev=audio0
		-serial stdio
		-vga std
		-m 4M
		-smp 4
		-no-shutdown
		-no-reboot
	)

	IF(CMAKE_BUILD_TYPE STREQUAL "Debug")
		SET(QEMU_DEBUG_FLAGS -d cpu_reset,int)
	ENDIF()

	ADD_CUSTOM_TARGET(
		qemu
		COMMAND qemu-system-i386 ${QEMU_FLAGS} ${QEMU_DEBUG_FLAGS} -drive
				file=${IMAGE_FILE},format=raw,if=ide,index=0
		DEPENDS image_file
	)

	ADD_CUSTOM_TARGET(
		bochs
		COMMAND echo c | bochs -q -f ${BOCHSRC_FILE}
		DEPENDS ${BOCHSRC_FILE}
	)
ENDIF()
