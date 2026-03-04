####################################
# CMakeLists.txt                   #
# Criado por Matheus Leme Da Silva #
####################################

set(BOOTSECT_ASM ${CMAKE_CURRENT_SOURCE_DIR}/source/bootsect.asm)
set(BOOTSECT_BIN ${CMAKE_CURRENT_BINARY_DIR}/bootsect.bin)

set(LOADER_ELF loader.elf)
set(LOADER_BIN ${CMAKE_CURRENT_BINARY_DIR}/loader.bin)

add_executable(${LOADER_ELF}
	source/entry.asm
	source/main.c
	source/disk.c
	source/vesa.c
	source/e820.c
	source/file.c
	source/util.c
	source/vga.c
	source/fat.c
)

target_link_libraries(${LOADER_ELF} PRIVATE libc)

if(ARCH STREQUAL "x86")
	target_compile_options(
		loader.elf PRIVATE
		$<$<COMPILE_LANGUAGE:C>:-m32 -march=i686 ${BASE_CFLAGS}>
		$<$<COMPILE_LANGUAGE:ASM_NASM>:-f elf32>
	)

	target_link_options(
		loader.elf PRIVATE
		-m32
		-nostdlib
		-static
		-T${CMAKE_CURRENT_SOURCE_DIR}/source/linker.ld
		-Wl,-melf_i386
		-Wl,--gc-sections
		)
endif()

add_custom_command(
	OUTPUT ${BOOTSECT_BIN}
	COMMAND ${CMAKE_ASM_NASM_COMPILER} -f bin ${BOOTSECT_ASM} -o ${BOOTSECT_BIN}
	DEPENDS ${BOOTSECT_ASM}
)

add_custom_command(
	OUTPUT ${LOADER_BIN}
	COMMAND ${CMAKE_OBJCOPY} -O binary ${LOADER_ELF} ${LOADER_BIN}
	DEPENDS ${LOADER_ELF}
)

add_custom_target(bootldr-bin
	COMMAND cat ${BOOTSECT_BIN} ${LOADER_BIN} > ${BOOTLDR_FILE}
	DEPENDS ${BOOTSECT_BIN} ${LOADER_BIN}
)
