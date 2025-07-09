;
; libasm for programs
;

USERSEG equ 0x1600

; 16-bit operations
global idiv_
idiv_:
	cwd
	idiv bx
ret

global idiv_u
idiv_u:
	xor dx, dx
	div bx
ret

global imod
imod:
	cwd
	idiv bx
	mov ax, dx
ret

global imodu
imodu:
	xor dx, dx
	div bx
	mov ax, dx
ret

global imul_, imul_u
imul_:
imul_u:
	imul bx
ret

global isl, islu
isl:
islu:
	mov cl, bl
	sar ax, cl
ret

global isru
isru:
	mov cl, bl
	shr ax, cl
ret

; 32-bit operations
global laddl, laddul
laddl:
laddul:
	add ax, [di]
	adc bx, [di+2]
ret

global landl, landul
landl:
landul:
	and ax, [di]
	and bx, [di+2]
ret

global lcmpl, lcmpul
lcmpl:
lcmpul:
	sub bx, [di+2]
	je .not_sure
ret
	.not_sure:
		cmp ax, [di]
		jb .b_and_lt
		jge .exit
		inc bx
	.exit: ret
	.b_and_lt:
		dec bx
ret

global ldecl, ldecul
ldecl:
ldecul:
	cmp word [bx], 0
	je .dec_both
	dec word[bx]
ret
	.dec_both:
		dec word[bx]
		dec word[bx+2]
ret

global lcoml, lcomul
lcoml:
lcomul:
	not ax
	not bx
ret

global lsubl, lsubul
lsubl:
lsubul:
	sub ax, [di]
	sbb ax, [di+2]
ret

global lnegl, lnegul
lnegl:
lnegul:
	neg bx
	neg ax
	sbb ax, 0
ret

global lorl, lorul

lorl:
lorul:
	or ax, [di]
	or bx, [di+2]
ret

global lincl, lincul
lincl:
lincul:
	inc word[bx]
	je .inc_high
	.inc_high:
		inc word[bx+2]
ret

global lmull, lmulul
lmull:
lmulul:
	mov cx, ax
	mul word[di+2]
	xchg ax, bx
	mul word[di]
	add bx, ax
	mov ax, [di]
	mul cx
	add bx, dx
ret

global lmodl
lmodl:
	mov cx, [di]
	mov di, [di+2]
	call ldivmod
ret

global lmodul
lmodul:
	mov cx, [di]
	mov di, [di+2]
	call ludivmod
ret

global ldivl
ldivl:
	mov cx, [di]
	mov di, [di+2]
	call ldivmod
	xchg ax, cx
	xchg bx, di
ret

global ldivul
ldivul:
	mov cx, [di]
	mov di, [di+2]
	call ludivmod
	xchg ax, cx
	xchg bx, di
ret

global lsll, lslul
lsll:
lslul:
	mov cx, di
	jcxz .exit
	cmp cx, 32
	jae .zero
	.loop:
		shl ax, 1
		rcl bx, 1
		loop .loop
	.exit: ret
	.zero:
		xor ax, ax
		mov bx, ax
ret

global lsrul
lsrul:
	mov cx, di
	jcxz .exit
	cmp cx, 32
	jae .zero
	.loop:
		shr bx, 1
		rcr ax, 1
		loop .loop
	.exit: ret
	.zero:
		xor ax, ax
		mov bx, ax
ret

global ltstl, ltstul
ltstl:
ltstul:
	test bx, bx
	je LTST_NOT_SURE
ret
LTST_NOT_SURE:
	test ax, ax
	js LTST_FIX_SIGN
ret
LTST_FIX_SIGN:
	inc bx
ret

global ldivmod
ldivmod:
	mov dx, di
	mov dl, bh
	test di, di
	jns .set_assign
	neg di
	neg cx
	sbb di, 0
	.set_assign:
		test bx, bx
		jns got_signs
		neg bx
		neg ax
		sbb bx, 0
		jmp got_signs

global ludivmod
ludivmod:
	xor dx, dx
got_signs:
	push bp
	push si
	mov bp, sp
	push di
	push cx

	test di, di
	jne divlarge
	test cx, cx
	je divzero
	cmp bx, cx
	jae divlarge
	xchg dx, bx
	div cx
	xchg cx, ax
	xchg ax, bx
	xchg ax, dx
	mov bx, di
	mov bx, di
	jmp zdivu1
divzero:
	test dl, dl
	jns return_result
	jmp negr
divlarge:
	push dx
	mov si, di
	mov dx, cx
	xor cx, cx
	mov di, cx
	cmp si, bx
	jb loop1
	ja zdivu
	cmp dx, ax
	ja zdivu
loop1:
	shl dx, 1
	rcl si, 1
	jc loop1_exit
	cmp si, bx
	jb loop1
	ja loop1_exit
	cmp dx, ax
	jbe loop1
loop1_exit:
	rcr si, 1
	rcr dx, 1
loop2:
	shl cx, 1
	rcl di, 1
	cmp si, bx
	jb loop2_over
	ja loop2_test
	cmp dx, ax
	ja loop2_test
loop2_over:
	add cx, 1
	add di, 0
	sub ax, dx
	sbb bx, si
loop2_test:
	shr si, 1
	rcr dx, 1
	cmp si, [bp-2]
	ja loop2
	jb zdivu
	cmp dx, [bp-4]
	jae loop2
zdivu:
	pop dx
zdivu1:
	test dh, dh
	js zbminus
	test dl, dl
	jns return_result
	mov dx, ax
	or dx, bx
	je negq
	sub ax, [bp-4]
	sbb bx, [bp-2]
	not cx
	not di
negr:
	neg bx
	neg ax
	sbb bx, 0
return_result:
	mov sp, bp
	pop si
	pop bp
ret
zbminus:
	test dl, dl
	js negr
	mov dx, ax
	or dx, bx
	je negq
	sub ax, [bp-4]
	sbb bx, [bp-2]
	not cx
	not si
	mov sp, bp
	pop si
	pop bp
ret
negq:
	neg di
	neg cx
	sbb di, 0
	mov bp, sp
	pop si
	pop bp
ret

global _syscall
_syscall:
  push bp
  mov bp, sp
  mov ax, [bp+4]  ; id
  mov bx, [bp+6]  ; arg1
  mov cx, [bp+8]  ; arg2
  mov dx, [bp+10] ; arg3
	mov di, [bp+12] ; arg4
	mov si, [bp+14] ; arg5
  int 0x80
  pop bp
ret

; void lwriteb(ushort seg, ushort off, uchar val);
global _lwriteb
_lwriteb:
	push bp
	mov bp, sp
	push es
	push ds
	mov ax, [bp+4] ; seg
	mov es, ax
	mov ds, ax
	mov di, [bp+6] ; off
	mov al, [bp+8] ; val
	mov [es:di], al
	pop ds
	pop es
	pop bp
ret

; void lwritew(ushort seg, ushort off, ushort val);
global _lwritew
_lwritew:
	push bp
	mov bp, sp
	push es
	push ds
	mov ax, [bp+4] ; seg
	mov es, ax
	mov ds, ax
	mov di, [bp+6] ; off
	mov ax, [bp+8] ; val
	mov [es:di], ax
	pop ds
	pop es
	pop bp
ret

; uchar lreadb(ushort seg, ushort off);
global _lreadb
_lreadb:
	push bp
	mov bp, sp
	push es
	push ds
	mov ax, [bp+4]
	mov es, ax
	mov ds, ax
	mov di, [bp+6]
	mov al, [es:di]
	xor ah, ah
	pop ds
	pop es
	pop bp
ret

; ushort lreadw(ushort seg, ushort off);
global _lreadw
_lreadw:
	push bp
	mov bp, sp
	push es
	push ds
	mov ax, [bp+4]
	mov es, ax
	mov ds, ax
	mov di, [bp+6]
	mov ax, [es:di]
	pop ds
	pop es
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
