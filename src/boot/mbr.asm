;
; bootsect.asm
; Created by Matheus Leme Da Silva
;
bits 16
org 0x7c00

jmp short start
nop

; Tample BPB(for old Computers)
db '????????'
dw 512
db 8
dw 1
db 4
dw 512
dw 0
db 0xf8
dw 32
secs_per_track dw 63
heads dw 16
dd 0
dd 0
drive db 0x80
db 0
db 0x29
dd 0x12345678
db '???????????'
db '????????'

start:
	cli
	xor ax, ax
	mov ds, ax
	mov es, ax
	mov ss, ax
	mov sp, 0x7bff
	sti

	mov [drive], dl

	push es
	mov ax, 0x9000
	mov es, ax
	xor bx, bx

	mov ah, 0x02
	mov al, 31
	mov ch, 0
	mov cl, 2
	mov dh, 0
	mov dl, [drive]
	int 0x13
	jc disk_error
	pop es
	mov dl, [drive]
	jmp 0x9000:0x0000

disk_error:
	mov si, err_msg
	.print:
		lodsb
		test al, al
		jz $
		mov ah, 0x0e
		int 0x10
		jmp .print

err_msg db 'ERROR: failed to load second bootloader stage!', 13, 10, 0

times 510-($-$$) db 0
dw 0xaa55
