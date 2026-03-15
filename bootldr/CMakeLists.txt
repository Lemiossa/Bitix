####################################
# CMakeLists.txt                   #
# Criado por Matheus Leme Da Silva #
####################################

SET(BOOTSECT_ASM ${CMAKE_CURRENT_SOURCE_DIR}/source/bootsect.asm)
SET(BOOTSECT_BIN ${CMAKE_CURRENT_BINARY_DIR}/bootsect.bin)

SET(LOADER_ELF loader.elf)
SET(LOADER_BIN ${CMAKE_CURRENT_BINARY_DIR}/loader.bin)

ADD_EXECUTABLE(
	${LOADER_ELF}
	source/entry.asm
	source/main.c
	source/disk.c
	source/vesa.c
	source/e820.c
	source/file.c
	source/util.c
	source/vga.c
	source/fat.c)

TARGET_LINK_LIBRARIES(${LOADER_ELF} PRIVATE libc)

TARGET_INCLUDE_DIRECTORIES(${LOADER_ELF}
						   PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/source)

IF(ARCH STREQUAL "x86")
	TARGET_COMPILE_OPTIONS(
		loader.elf
		PRIVATE $<$<COMPILE_LANGUAGE:C>:-m32 -march=i686 ${BASE_CFLAGS}>
				$<$<COMPILE_LANGUAGE:ASM_NASM>:-f elf32>)

	TARGET_LINK_OPTIONS(
		loader.elf
		PRIVATE
		-m32
		-nostdlib
		-static
		-T${CMAKE_CURRENT_SOURCE_DIR}/source/linker.ld
		-Wl,-melf_i386
		-Wl,--gc-sections)
ENDIF()

ADD_CUSTOM_COMMAND(
	OUTPUT ${BOOTSECT_BIN}
	COMMAND ${CMAKE_ASM_NASM_COMPILER} -f bin ${BOOTSECT_ASM} -o ${BOOTSECT_BIN}
	DEPENDS ${BOOTSECT_ASM})

ADD_CUSTOM_COMMAND(
	OUTPUT ${LOADER_BIN}
	COMMAND ${CMAKE_OBJCOPY} -O binary ${LOADER_ELF} ${LOADER_BIN}
	DEPENDS ${LOADER_ELF})

ADD_CUSTOM_TARGET(
	bootldr-bin
	COMMAND cat ${BOOTSECT_BIN} ${LOADER_BIN} > ${BOOTLDR_FILE}
	DEPENDS ${BOOTSECT_BIN} ${LOADER_BIN})
