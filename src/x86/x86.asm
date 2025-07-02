;
; x86.asm - Assembly low level functions
; Created by Matheus Leme Da Silva
;
bits 16

; void putc(uchar c);
extern _current_attr
extern _screen_width, _screen_height
extern _video_mode
global _putc
_putc:
	push bp
	mov bp, sp
	push ax
	push bx
	push cx
	push dx
	push si

	mov al, [_video_mode]
	cmp al, 1
	jne .done

	mov ah, 0x03
	mov bh, 0x00
	int 0x10

	mov si, dx

	mov al, [bp+4] ; c

	; Special chars
	cmp al, 0x08
	je .backspace
	cmp al, 0x0a
	je .newline
	cmp al, 0x0d
	je .carriage

	; print char
	mov ah, 0x09
	mov bh, 0x00
	mov bl, [_current_attr]
	mov cx, 1
	int 0x10

	; Advance the cursor
	mov dx, si
	inc dl
	mov al, [_screen_width]
	dec al
	cmp dl, al
	jbe .set_cursor
	mov dl, 0
	inc dh

	; _putc.checkscroll - check the scroll and scroll
	.checkscroll:
		mov al, [_screen_height]
		dec al
		cmp dh, al
		jb .set_cursor

		mov ax, 0x0601
		mov bh, [_current_attr]
		mov cx, 0x0000
		mov dl, [_screen_width]
		dec dl
		mov dh, [_screen_height]
		dec dh
		int 0x10

		mov dh, [_screen_height]
		dec ah
		mov dl, 0

	; _putc.set_cursor - update cursor position
	.set_cursor:
		mov ah, 0x02
		mov bh, 0x00
		int 0x10
		jmp .done

	; _putc.backspace - execute \b
	.backspace:
		mov dx, si
		cmp dl, 0
		jne .bs_no_line_wrap
		cmp dh, 0
		je .done
		dec dh
		mov al, [_screen_width]
		dec al
		mov dl, al
		jmp .bs_draw
		.bs_no_line_wrap: dec dl
		.bs_draw:
			mov ah, 0x02
			mov bh, 0x00
			int 0x10

			mov ah, 0x09
			mov al, ' '
			mov bl, [_current_attr]
			mov cx, 1
			int 0x10

			mov ah, 0x02
			mov bh, 0x00
			int 0x10
			jmp .done

	; _putc.newline - move cursor to new line
	.newline:
		mov dx, si
		inc dh
		mov dl, 0
		mov al, [_screen_height]
		dec al
		cmp dh, al
		jae .checkscroll
		jmp .set_cursor

	; _putc.carriage - move cursor to start of line
	.carriage:
		mov dx, si
		mov dl, 0
		jmp .set_cursor

	; _putc.done - end of function
	.done:
		pop si
		pop dx
		pop cx
		pop bx
		pop ax
		pop bp
ret

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
	mov dx, ds

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

; void updateregs(regs16_t *in);
global _updateregs
_updateregs:
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
