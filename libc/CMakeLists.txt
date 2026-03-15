####################################
# CMakeLists.txt                   #
# Criado por Matheus Leme Da Silva #
####################################

ADD_LIBRARY(libc STATIC source/string.c source/stdio.c source/stdlib.c)

IF(ARCH STREQUAL "x86")
	TARGET_COMPILE_OPTIONS(
		libc PRIVATE $<$<COMPILE_LANGUAGE:C>:-m32 -march=i686 ${BASE_CFLAGS}>
					 $<$<COMPILE_LANGUAGE:ASM_NASM>:-f elf32>)
ENDIF()
