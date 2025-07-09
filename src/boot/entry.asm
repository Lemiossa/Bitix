;
; entry.asm
; Created by Matheus Leme Da Silva
;
bits 16

global _start
extern _main

BOOT_SEG equ 0x9000

global _drive
_drive db 0x00

_start:
	cli
	mov ax, BOOT_SEG
	mov ds, ax
	mov es, ax
	mov ss, ax
	mov sp, STK
	sti
	mov [_drive], dl

	cli
	mov ax, 0
	mov es, ax

	mov ax, [es:0x08*4]
	mov [irq0_old_off], ax
	mov ax, [es:0x08*4+2]
	mov [irq0_old_seg], ax
	mov word[es:0x08*4], irq0_handler
	mov word[es:0x08*4+2], BOOT_SEG

	mov al, 0x36
	out 0x43, al
	mov ax, 11931
	out 0x40, al
	mov al, ah
	out 0x40, al
	
	sti

	call _main

	jmp $

irq0_old_seg dw 0
irq0_old_off dw 0
global _ticks
_ticks dd 0

irq0_handler:
	pusha
	push ds

	mov ax, BOOT_SEG
	mov ds, ax

	inc dword[_ticks]
	.done:
		mov al, 0x20
		out 0x20, al

		pop ds
		popa
iret

section .bss
align 16
	STKB:
		resb 4096
	STK:
