####################################
# CMakeLists.txt                   #
# Criado por Matheus Leme Da Silva #
####################################

add_executable(kernel.elf
	source/arch/${ARCH}/entry.asm
	source/arch/${ARCH}/intr.asm
	source/arch/${ARCH}/gdt.c
	source/arch/${ARCH}/idt.c
	source/arch/${ARCH}/pmm.c
	source/graphics.c
	source/terminal.c
	source/timer.c
	source/main.c
	source/pic.c
	source/pit.c
	source/vga.c
	source/kbd.c
)

target_link_libraries(kernel.elf PRIVATE libc)

target_include_directories(kernel.elf PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/include/arch/${ARCH}
	${CMAKE_CURRENT_SOURCE_DIR}/include
)

if(ARCH STREQUAL "x86")
	target_compile_options(
		kernel.elf PRIVATE
		$<$<COMPILE_LANGUAGE:C>:-m32 -march=i686 ${BASE_CFLAGS}>
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

add_custom_target(kernel_bin
	COMMAND ${CMAKE_OBJCOPY}
			-O binary
			kernel.elf
			${KERNEL_FILE}

	DEPENDS kernel.elf
)
