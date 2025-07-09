;
; entry.asm
; Created by Matheus Leme Da Silva
;
bits 16
extern _kmain

global _drive
_drive db 0

KSEG equ 0x0600
global _start
_start:
	cli
	mov ax, KSEG
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

global KSTK
section .bss
stack:
	resb 4096
KSTK:
