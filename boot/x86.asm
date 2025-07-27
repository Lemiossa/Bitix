; x86.asm
; Created by Matheus Leme Da Silva
; ---------------------------------
section .text
extern drive

; Inspired by:
; https://github.com/nanobyte-dev/nanobyte_os/blob/master/src/bootloader/stage2/x86.asm

%macro ermode 0
	bits 32
	
	jmp word 0x18:.pmode16

	.pmode16:
		bits 16
		mov eax, cr0
		and eax, ~1
		mov cr0, eax

		jmp word 0x00:.rmode
	.rmode:
		mov ax, 0
		mov ds, ax
		mov ss, ax

		sti
%endmacro

%macro epmode 0
	cli

	mov eax, cr0
	or eax, 1
	mov cr0, eax

	jmp dword 0x08:.pmode

	.pmode:
		mov ax, 0x10
		mov ds, ax
		mov ss, ax
%endmacro

; void io_putc(char c);
global io_putc
io_putc:
	bits 32
	push ebp
	mov ebp, esp
	
	ermode
	bits 16

	mov al, byte[ebp+8]
	mov ah, 0x0e
	int 0x10

	epmode
	bits 32
	
	pop ebp
	ret

; uint8_t io_readsectors(uint64_t lba, uint16_t seg, uint16_t off, uint16_t sectors);
global io_readsectors
io_readsectors:
	bits 32
	push ebp
	mov ebp, esp

	ermode
	bits 16
	cli
	mov eax, [ebp+8]
	mov dword[dap.lba], eax
	mov eax, [ebp+12]
	mov dword[dap.lba+4], eax
	
	mov ax, word[ebp+16]
	mov word[dap.seg], ax
	mov ax, word[ebp+20]
	mov word[dap.off], ax

	mov ax, word[ebp+24]
	mov word[dap.scs], ax

	xor esi, esi
	xor eax, eax
	xor edx, edx

	mov si, dap
	mov dl, [drive]
	mov ah, 0x42
	int 0x13

	mov bl, ah

	jmp .done
	.done:
		sti
		epmode
		bits 32

		xor eax, eax
		mov al, bl

		pop ebp
		ret

; uint8_t io_disk_reset();
global io_disk_reset
io_disk_reset:
	bits 32
	ermode
	bits 16

	mov ah, 0x00
	mov dl, [drive]
	int 0x13

	mov bl, ah
	mov ecx, 0xffff
.delay:
	nop
	loop .delay
	
	epmode
	bits 32

	xor eax, eax
	mov al, bl
	ret

align 16
dap:
	.size db 0x10
	.pad db 0
	.scs dw 0
	.off dw 0
	.seg dw 0
	.lba dq 0
