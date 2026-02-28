add_executable(loader.elf
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

target_link_libraries(loader.elf PRIVATE libc)

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
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/bootsect.bin
	COMMAND ${CMAKE_ASM_NASM_COMPILER} -f bin
			${CMAKE_CURRENT_SOURCE_DIR}/source/bootsect.asm
			-o ${CMAKE_CURRENT_BINARY_DIR}/bootsect.bin
	DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/source/bootsect.asm
)

add_custom_command(
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/loader.bin
	COMMAND ${CMAKE_OBJCOPY}
			-O binary
			loader.elf
			${CMAKE_CURRENT_BINARY_DIR}/loader.bin
	DEPENDS loader.elf
)

add_custom_command(
	OUTPUT ${BOOTLDR_FILE}
	COMMAND cat
			${CMAKE_CURRENT_BINARY_DIR}/bootsect.bin
			${CMAKE_CURRENT_BINARY_DIR}/loader.bin
			> ${BOOTLDR_FILE}
	DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/bootsect.bin
			${CMAKE_CURRENT_BINARY_DIR}/loader.bin
)

add_custom_target(bootldr_bin
	DEPENDS ${BOOTLDR_FILE}
)
