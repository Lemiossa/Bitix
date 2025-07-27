bits 32
global _start
extern kmain

_start:
	mov esp, stack
	call kmain
	jmp $

section .bss
align 16
stack_bottom: resb 0x4000
stack:
