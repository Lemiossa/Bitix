;
; x86.asm - Assembly low level functions
; Created by Matheus Leme Da Silva
;
bits 16

%macro OUTB 2
out %1, %2
%endmacro

%macro INB 2
in %1, %2
%endmacro

%macro PIC_SEND_EOI 0
mov al, 0x20
out 0x20, al
%endmacro
%macro PIC_SEND_EOI_MOUSE 0
mov al, 0x20
out 0xa0, al
out 0x20, al
%endmacro

KSEG equ 0x0600

; void outportb(ushort port, uchar value);
global _outportb
_outportb:
	push bp
	mov bp, sp
	mov dx, [bp+4] ; port
	mov al, [bp+6] ; value
	out dx, al
	pop bp
ret

; uchar inportb(ushort port);
global _inportb
_inportb:
	push bp
	mov bp, sp
	mov dx, [bp+4] ; port
	in al, dx
	xor ah, ah
	pop bp
ret

; void setvect(uchar intnum, void (*handler)());
global _setvect
_setvect:
	push bp
	mov bp, sp

	push ax
	push bx
	push dx
	push es
	push di

	mov al, [bp+4]
	mov bx, [bp+6]
	mov dx, cs

	xor ah, ah
	shl ax, 2
	mov di, ax

	xor ax, ax
	mov es, ax

	mov [es:di], bx
	mov [es:di+2], dx

	pop di
	pop es
	pop dx
	pop bx
	pop ax
	pop bp
ret

; void setregs(regs16_t *in);
global _setregs
_setregs:
	push bp
	mov bp, sp

	mov si, [bp+4]
	mov di, .regs

	mov cx, 6
	.loop:
		mov ax, [si]
		mov [di], ax
		add si, 2
		add di, 2
		loop .loop

		mov ax, [.regs]
		mov bx, [.regs+2]
		mov cx, [.regs+4]
		mov dx, [.regs+6]
		mov si, [.regs+8]
		mov di, [.regs+10]
	pop bp
ret
.regs: times 6 dw 0

; void getregs(regs16_t *out);
global _getregs
_getregs:
	push bp
	mov bp, sp

	push di
	mov di, [bp+4]
	mov [di], ax
	mov [di+2], bx
	mov [di+4], cx
	mov [di+6], dx
	mov [di+8], si
	mov [di+10], di
	pop di
	pop bp
ret

; void lwriteb(ushort seg, ushort off, uchar val);
global _lwriteb
_lwriteb:
	push bp
	mov bp, sp
	push es
	mov ax, [bp+4] ; seg
	mov es, ax
	mov di, [bp+6] ; off
	mov al, [bp+8] ; val
	mov [es:di], al
	pop es
	pop bp
ret

; void lwritew(ushort seg, ushort off, ushort val);
global _lwritew
_lwritew:
	push bp
	mov bp, sp
	push es
	mov ax, [bp+4] ; seg
	mov es, ax
	mov di, [bp+6] ; off
	mov ax, [bp+8] ; val
	mov [es:di], ax
	pop es
	pop bp
ret

; uchar lreadb(ushort seg, ushort off);
global _lreadb
_lreadb:
	push bp
	mov bp, sp
	push es
	mov ax, [bp+4]
	mov es, ax
	mov di, [bp+6]
	mov al, [es:di]
	xor ah, ah
	pop es
	pop bp
ret

; ushort lreadw(ushort seg, ushort off);
global _lreadw
_lreadw:
	push bp
	mov bp, sp
	push es
	mov ax, [bp+4]
	mov es, ax
	mov di, [bp+6]
	mov ax, [es:di]
	pop es
	pop bp
ret

; void io_init_pit(uint divisor);
global _io_init_pit
_io_init_pit:
	push bp
	mov bp, sp
	push ax
	push bx
	push dx
	push cx

	push es
	xor ax, ax
	mov es, ax
	mov word[es:0x20*4], irq0_handler
	mov bx, cs
	mov word[es:0x20*4+2], bx
	pop es

	mov ax, [bp+4] ; freq
	cmp ax, 0
	je .end
	mov bx, ax

	mov al, 0x36
	out 0x43, al
	mov al, bl
	out 0x40, al
	mov al, bh
	out 0x40, al

	.end:
		pop cx
		pop dx
		pop bx
		pop ax
		pop bp
ret

global _ticks
_ticks dd 0

global irq0_handler
irq0_handler:
	pusha
	inc dword[_ticks]
	PIC_SEND_EOI
	popa
iret

; void init_mouse();
global _init_mouse
_init_mouse:
	push es
	mov ax, 0
	mov es, ax
	mov word[es:0x2c*4], irq12_handler
	mov word[es:0x2c*4+2], KSEG
	pop es

	; Unlock masks
	;irq2
	in al, 0x21
	and al, 11111011b
	out 0x21, al
	;irq12
	in al, 0xa1
	and al, 11101111b
	out 0xa1, al
ret

global irq12_handler
irq12_handler:
	pusha

	in al, 0x60

    mov ah, 0x0e
    mov bx, 0x0007
    int 0x10

	PIC_SEND_EOI_MOUSE
	popa
iret

global _disk
_disk:
	.drive db 0x80
	.spt db 63
	.heads db 16
	.label times 4 db 0

; int io_init_disk(uchar drive);
global _io_init_disk
_io_init_disk:
	push bp
	mov bp, sp

	mov ax, [bp+4]
	mov [_disk.drive], ax

	mov ah, 0x08
	mov dl, [bp+4] ; drive
	int 0x13
	jc .fail

	test cl, cl
	jz .fail

	inc dh
	mov [_disk.heads], dh

	and cl, 0x3f
	mov [_disk.spt], cl

	xor ax, ax
	pop bp
ret
	.fail:
		mov ax, 1
		pop bp
ret

; int io_readblock_chs(u16 head, u16 track, u16 sector, void *buf);
global _io_readblock_chs
_io_readblock_chs:
	push bp
	mov bp, sp
	push bx
	push cx
	push dx
	push si
	push es
	push ds

	mov ah, 0x02 ; read sectors func
	mov al, 1 ; read 1 sector
	mov ch, [bp+6] ; track
	mov cl, [bp+8] ; sector
	mov dh, [bp+4] ; head
	mov dl, [_disk.drive] ; drive num

	; buffer
	mov si, [bp+10] ; buffer
	mov bx, ds
	mov es, bx
	mov bx, si

	stc
	int 0x13
	jc .fail
	.success:
		xor ax, ax
		jmp .done
	.fail:
		mov ax, 1
	.done:
		pop ds
		pop es
		pop si
		pop dx
		pop cx
		pop bx
		pop bp
ret

; int io_writeblock_chs(u16 head, u16 track, u16 sector, void *buf);
global _io_writeblock_chs
_io_writeblock_chs:
	push bp
	mov bp, sp
	push bx
	push cx
	push dx
	push si
	push es
	push ds

	mov ah, 0x03 ; write sectors func
	mov al, 1 ; write 1 sector
	mov ch, [bp+6] ; track
	mov cl, [bp+8] ; sector
	mov dh, [bp+4] ; head
	mov dl, [_disk.drive] ; drive num

	; buffer
	mov si, [bp+10] ; buffer
	mov bx, ds
	mov es, bx
	mov bx, si

	stc
	int 0x13
	jc .fail
	.success:
		xor ax, ax
		jmp .done
	.fail:
		mov ax, 1
	.done:
		pop ds
		pop es
		pop si
		pop dx
		pop cx
		pop bx
		pop bp
ret

; void reset_disk();
global _reset_disk
_reset_disk:
	push ax
	push dx
	mov ah, 0x00
	mov dl, [_disk.drive]
	stc
	int 0x13
	pop dx
	pop ax
ret

; int lcall(ushort seg, ushort off);
global _lcall
_lcall:
    push bp
    mov bp, sp

    mov ax, [bp+6]   ; offset
    mov bx, [bp+4]   ; segment

	pusha
    push es
    push ds
    
    mov ds, bx
    mov es, bx

	mov si, return_point
	mov di, cs

	push bx
	push ax
	mov ax, [bp+4]
	mov bx, cs
	retf

return_point:
	mov [.return], ax
    pop ds
    pop es
    popa

	mov ax, [.return]
    pop bp
ret
.return dw 0

section .data
sys_result dw 0
sys_id dw 0
sys_arg1 dw 0
sys_arg2 dw 0
sys_arg3 dw 0
sys_arg4 dw 0
sys_arg5 dw 0
sys_old_stack_segment dw 0
sys_old_stack_pointer dw 0
section .bss
sys_stkb: resb 4096
sys_stk:
section .text

; sys_isr
global _sys_isr
extern _syscall
_sys_isr:
	cli
	pusha
	push es
	push ds

	push di
	mov di, KSEG
	mov ds, di
	mov es, di
	pop di

	mov [sys_id], ax
	mov [sys_arg1], bx
	mov [sys_arg2], cx
	mov [sys_arg3], dx
	mov [sys_arg4], di
	mov [sys_arg5], si

	mov di, ss
	mov [sys_old_stack_segment], di
	mov [sys_old_stack_pointer], sp

	cli
	mov di, KSEG
	mov ss, di
	mov sp, sys_stk
	sti

	mov ax, [sys_arg5]
	push ax
	mov ax, [sys_arg4]
	push ax
	mov ax, [sys_arg3]
	push ax
	mov ax, [sys_arg2]
	push ax
	mov ax, [sys_arg1]
	push ax
	mov ax, [sys_id]
	push ax
	sti
	call _syscall
	cli
	add sp, 8

	mov [sys_result], ax

	cli
	mov di, [sys_old_stack_segment]
	mov ss, di
	mov sp, [sys_old_stack_pointer]
	sti

.done:
	pop ds
	pop es
	popa
	sti
	mov ax, [sys_result]
iret

; int io_set_vesa_mode(ushort mode);
global _io_set_vesa_mode
_io_set_vesa_mode:
	push bp
	mov bp, sp
	cli
	cld

	mov ax, 0x4f02
	mov bx, [bp+4]
	int 0x10

	.done:
		sti
		pop bp
ret

; int io_set_vesa_bank(uint bank);
global _io_set_vesa_bank
_io_set_vesa_bank:
	push bp
	mov bp, sp
	cli
	cld

	mov ax, 0x4f05
	mov bx, 0
	mov dx, [bp+4]
	int 0x10

	cmp ax, 0x004f
	je .success

	mov ax, 1 ; return 1
	jmp .exit
	.success:
		mov ax, 0 ; return 0
	.exit:
		sti
		pop bp
ret
