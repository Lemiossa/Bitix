add_library(libc STATIC
	source/string.c
	source/stdio.c
	source/stdlib.c
)

if(ARCH STREQUAL "x86")
target_compile_options(
	libc PRIVATE
	$<$<COMPILE_LANGUAGE:C>:-m32 -march=i686 ${BASE_CFLAGS}>
	$<$<COMPILE_LANGUAGE:ASM_NASM>:-f elf32>
)
endif()

