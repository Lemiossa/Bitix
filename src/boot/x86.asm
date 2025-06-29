;
; x86.asm
; Created by Matheus Leme Da Silva
;
bits 16

; void lwrite8(u16 seg, u16 ofs, u8 val);
global _lwrite8
_lwrite8:
	push bp
	mov bp, sp
	push es
	mov ax, [bp+4] ; seg
	mov es, ax
	mov bx, [bp+6] ; ofs
	mov al, [bp+8] ; val
	mov [es:bx], al
	pop es
	pop bp
ret

; u8 lmem_read8(u16 seg, u16 ofs);
global _lread8
_lread8:
	push bp
	mov bp, sp
	push es
	mov ax, [bp+4] ; seg
	mov es, ax
	mov bx, [bp+6] ; ofs
	mov al, [es:bx]
	pop es
	pop bp
ret

; void lwrite16(u16 seg, u16 ofs, u16 val);
global _lwrite16
_lwrite16:
	push bp
	mov bp, sp
	push es
	mov ax, [bp+4] ; seg
	mov es, ax
	mov bx, [bp+6] ; ofs
	mov ax, [bp+8] ; val
	mov [es:bx], ax
	pop es
	pop bp
ret

; u16 lread16(u16 seg, u16 ofs);
global _lread16
_lread16:
	push bp
	mov bp, sp
	push es
	mov ax, [bp+4] ; seg
	mov es, ax
	mov bx, [bp+6] ; ofs
	mov ax, [es:bx]
	pop es
	pop bp
ret

; void ljump(u16 seg, u16 off);
global _ljump
_ljump:
  push bp
  mov bp, sp
  mov ax, [bp+4] ; segment
  mov dx, [bp+6] ; offset
  mov [farptr], dx
  mov [farptr+2], ax
  jmp far [farptr]
	mov ax, 1 ; if error
  pop bp
ret
section .bss
farptr: resw 0
section .text

; void lcall(u16 seg, u16 off)
global _lcall
_lcall:
  push bp
  mov bp, sp
  mov ax, [bp+4] ; segment
  mov dx, [bp+6] ; offset
  mov [farptr], dx
  mov [farptr+2], ax
  call far [farptr]
  pop bp
ret
section .bss
farptr: resw 0
section .text

; void hlt();
global _hlt
_hlt:
	hlt
ret

; void sti();
global _sti
_sti:
	sti
ret

; void cli();
global _cli
_cli:
	cli
ret

; void outb(u16 port, u8 val);
global _outb
_outb:
	push bp
	mov bp, sp
	mov dx, [bp+4]
	mov al, [bp+6]
	out dx, al
	pop bp
ret

; u8 inb(u16 port);
global _inb
_inb:
	push bp
	mov bp, sp
	mov dx, [bp+4]
	in al, dx
	pop bp
ret

; void outw(u16 port, u16 val);
global _outw
_outw:
	push bp
	mov bp, sp
	mov dx, [bp+4] ; port
	mov ax, [bp+6] ; val
	out dx, ax
	pop bp
ret

; u16 inw(u16 port);
global _inw
_inw:
	push bp
	mov bp, sp
	mov dx, [bp+4]
	in ax, dx
	pop bp
ret

; void outl(u16 port, u32 val);
global _outl
_outl:
	push bp
	mov bp, sp
	mov dx, [bp+4]
	mov eax, [bp+6]
	out dx, eax
	pop bp
ret


; u32 inl(u16 port);
global _inl
_inl:
	push bp
	mov bp, sp
	mov dx, [bp+4]
	in eax, dx
	mov edx, eax
	shr edx, 16
	pop bp
ret

; void io_set_video_mode(u8 mode);
global _io_set_video_mode
_io_set_video_mode:
	push bp
	mov bp, sp
	push ax
	mov ah, 0x00
	mov al, [bp+4]
	int 0x10
	pop ax
	pop bp
ret

; int io_key_pressed(void);
global _io_key_pressed
_io_key_pressed:
	in al, 0x64
	test al, 1
	jz .no_key
	mov ax, 1
ret
	.no_key:
		xor ax, ax
ret

; uchar io_get_key(void);
global _io_get_key
_io_get_key:
	.wait_key:
		in al, 0x64
		test al, 1
		jz .wait_key

		in al, 0x60
		mov ah, 0
ret

; void putc(uchar c);
global _putc
_putc:
	push bp
	mov bp, sp
	push ax
	mov ah, 0x0e
	mov al, [bp+4]
	int 0x10
	pop ax
	pop bp
ret

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

; int io_readblock_chs(uchar head, uchar track, uchar sector, void *buf);
global _io_readblock_chs
_io_readblock_chs:
	push bp
	mov bp, sp
	push bx
	push cx
	push dx
	push si

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
		pop si
		pop dx
		pop cx
		pop bx
		pop bp
ret
.retries db 0

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

; void setax(u16 val);
global _setax
_setax:
	push bp
	mov bp, sp
	mov ax, [bp+4]
	pop bp
ret

; void setbx(u16 val);
global _setbx
_setbx:
	push bp
	mov bp, sp
	mov bx, [bp+4]
	pop bp
ret

; void setcx(u16 val);
global _setcx
_setcx:
	push bp
	mov bp, sp
	mov cx, [bp+4]
	pop bp
ret

; void setdx(u16 val);
global _setdx
_setdx:
	push bp
	mov bp, sp
	mov dx, [bp+4]
	pop bp
ret

; void get_low_memory(void);
global _get_low_memory
_get_low_memory:
	push bp
	mov bp, sp
	mov ah, 0x88
	int 0x15
	jnc .done
	mov ax, 512
	.done:
		pop bp
ret
