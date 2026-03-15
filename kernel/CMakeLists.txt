####################################
# CMakeLists.txt                   #
# Criado por Matheus Leme Da Silva #
####################################

ADD_EXECUTABLE(
	kernel.elf
	source/arch/${ARCH}/entry.asm
	source/arch/${ARCH}/intr.asm
	source/arch/${ARCH}/cpuid.asm
	source/arch/${ARCH}/cpuid.c
	source/arch/${ARCH}/acpi.c
	source/arch/${ARCH}/gdt.c
	source/arch/${ARCH}/idt.c
	source/arch/${ARCH}/fpu.c
	source/arch/${ARCH}/pmm.c
	source/arch/${ARCH}/vmm.c
	source/graphics.c
	source/terminal.c
	source/timer.c
	source/debug.c
	source/sched.c
	source/panic.c
	source/heap.c
	source/main.c
	source/util.c
	source/pic.c
	source/pit.c
	source/pci.c
	source/ata.c
	source/fat.c
	source/vga.c
	source/vfs.c
)

TARGET_LINK_LIBRARIES(kernel.elf PRIVATE libc)

TARGET_INCLUDE_DIRECTORIES(
	kernel.elf PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/arch/${ARCH}
					   ${CMAKE_CURRENT_SOURCE_DIR}/include)

IF(ARCH STREQUAL "x86")
	TARGET_COMPILE_OPTIONS(
		kernel.elf
		PRIVATE $<$<COMPILE_LANGUAGE:C>:-m32
				-march=i686
				-O2
				${BASE_CFLAGS}
				-mno-sse
				-mno-sse2
				-mfpmath=387>
				$<$<COMPILE_LANGUAGE:ASM_NASM>:-f
				elf32>)

	TARGET_LINK_OPTIONS(
		kernel.elf
		PRIVATE
		-m32
		-static
		-nostdlib
		-T${CMAKE_CURRENT_SOURCE_DIR}/source/arch/${ARCH}/linker.ld
		-Wl,-melf_i386
		-Wl,-Map=${KERNEL_MAP_FILE}
		-Wl,--gc-sections)
ENDIF()

ADD_CUSTOM_TARGET(
	kernel-bin
	COMMAND ${CMAKE_OBJCOPY} -O binary kernel.elf ${KERNEL_FILE}
	DEPENDS kernel.elf)
