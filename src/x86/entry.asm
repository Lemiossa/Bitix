;
; entry.asm
; Created by Matheus Leme Da Silva
;
bits 16
extern _kmain

global _drive
_drive db 0

global _start
_start:
	cli
	mov ax, 0x1000
	mov ds, ax
	mov es, ax
	mov ss, ax
	mov sp, KSTK
	sti

	; Save the drive
	mov [_drive], bl

	; Enter in text mode
	mov ax, 0x0003
	int 0x10

	; Call kmain
	call _kmain

	; If stay here, an fatal error ocurred
hang:
	hlt
	jmp hang

section .bss
align 8
	stack:
		resb (1024*8)
	KSTK:
