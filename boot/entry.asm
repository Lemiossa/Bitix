section .text
global _start
extern main

bits 16
_start:
	cli
	xor ax, ax
	mov ds, ax
	mov es, ax
	mov ax, 0x9000
	mov ss, ax
	mov sp, 0xffff
	sti

	mov [drive], dl

	in al, 0x92
	or al, 2
	out 0x92, al

	xor eax, eax

	cli
	lgdt[gdt_descriptor]
	
	mov eax, cr0
	or eax, 1
	mov cr0, eax
	
	jmp CODESEG32:pmode
	
bits 32
pmode:
	mov ax, DATASEG32
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax

	mov esp, stack

	call main
	jmp $
bits 16

global drive
drive db 0

gdt_start:
gdt_null: 
	dq 0
gdt_code32:
	dw 0xffff
	dw 0x0000
	db 0x00
	db 10011010b
	db 11001111b
	db 0x00
gdt_data32:
	dw 0xffff
	dw 0x0000
	db 0x00
	db 10010010b
	db 11001111b
	db 0x00
gdt_code16:
	dw 0xffff
	dw 0x0000
	db 0x00
	db 10011010b
	db 00001111b
	db 0x00
gdt_data16:
	dw 0xffff
	dw 0x0000
	db 0x00
	db 10010010b
	db 00001111b
	db 0x00
gdt_end:
gdt_descriptor:
	dw gdt_end - gdt_start - 1
	dd gdt_start

CODESEG32 equ 0x08
DATASEG32 equ CODESEG32+0x08
CODESEG16 equ DATASEG32+0x08
DATASEG16 equ CODESEG16+0x08
section .bss
stack_bottom: resb 0x4000
stack:
