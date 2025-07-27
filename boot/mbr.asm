; mbr.asm
; Created by Matheus Leme Da Silva
; ---------------------------------
bits 16
org 0
section .text

BOOT_LOAD_ADDR equ 0x00000500
SECTORS_TO_LOAD equ 59 ; confirms that the second stage has exactly that, a maximum of 64 sectors

jmp short start
nop

; Fake BPB for old PCs
db 'BITIX OS'
dw 512
db 8
dw 1
db 4
dw 512
dw 0
db 0xf8
dw 32
dw 63
dw 16
dd 0
dd 0
db 0x80
db 0
db 0x29
dd 0x12345678
db 'BITIXOS    '
db 'BFX     '

start:
    cli
    mov ax, 0x7c0
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ax, 0x0000
    mov ss, ax
    mov sp, 0xffff
    sti
    ; Save the drive
    mov [drive], dl
    mov ax, 0x0003
    int 0x10
    ; loads the second stage
    push es
    mov ax, (BOOT_LOAD_ADDR>>4)
    mov es, ax
    mov bx, (BOOT_LOAD_ADDR&0xf)
    mov ah, 0x02
    mov al, SECTORS_TO_LOAD
    mov ch, 0
    mov cl, 2
    mov dh, 0
    mov dl, [drive]
    int 0x13
    pop es
    jc error
    ; put the drive in dl and skip using return far
    mov dl, [drive]
    push word (BOOT_LOAD_ADDR>>4)
    push word (BOOT_LOAD_ADDR&0xf)
    retf

    ; just in case of shit
    jmp hang
error:
    mov si, msg_error
    call puts
hang:
    hlt
    jmp hang
puts:
    push ax
    push si
    .loop:
        lodsb
        test al, al
        jz .done
        mov ah, 0x0e
        int 0x10
        jmp .loop
    .done:
        pop si
        pop ax
ret

drive db 0
msg_error db 'Failed to load second stage!', 13, 10, 0

times 510-($-$$) db 0
dw 0x55aa
