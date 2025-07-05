;
; start for programs
; 
bits 16
extern _main

global _start
_start:
	mov ds, ax
	mov es, ax
	mov ss, ax
	mov sp, STK
	call _main
retf

section .bss
align 8
STKB:
	resb (1024*2)
STK:

