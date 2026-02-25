add_executable(kernel.elf
	source/arch/${ARCH}/entry.asm
	source/arch/${ARCH}/intr.asm
	source/arch/${ARCH}/gdt.c
	source/arch/${ARCH}/idt.c
	source/graphics.c
	source/terminal.c
	source/timer.c
	source/main.c
	source/pic.c
	source/pit.c
	source/vga.c
)

target_link_libraries(kernel.elf PRIVATE libc)

set(INCLUDES
	-I${CMAKE_CURRENT_SOURCE_DIR}/include/arch/${ARCH}
	-I${CMAKE_CURRENT_SOURCE_DIR}/include
)

if(ARCH STREQUAL "x86")
	target_compile_options(
		kernel.elf PRIVATE
		$<$<COMPILE_LANGUAGE:C>:-m32 -march=i686 ${BASE_CFLAGS} ${INCLUDES}>
		$<$<COMPILE_LANGUAGE:ASM_NASM>:-f elf32>
	)

	target_link_options(
		kernel.elf PRIVATE
		-m32
		-nostdlib
		-T${CMAKE_CURRENT_SOURCE_DIR}/source/arch/${ARCH}/linker.ld
		-Wl,-melf_i386
		-Wl,-static
		-Wl,--gc-sections
		)
endif()

add_custom_command(
	OUTPUT ${KERNEL_FILE}
	COMMAND ${CMAKE_OBJCOPY}
			-O binary
			kernel.elf
			${KERNEL_FILE}
	DEPENDS kernel.elf
)

add_custom_target(kernel
	DEPENDS ${KERNEL_FILE}
)
