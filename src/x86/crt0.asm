global _start
extern ___mkargv
_start:
	mov word[return_point], si
	mov word[return_point+2], di
	mov word[__seg], ax
	mov word[__old_seg], bx
	mov ax, ss
	mov word[__old_ss], ax
	mov ax, sp
	mov word[__old_sp], ax
	cli
	mov ax, [__seg]
	mov ss, ax
	mov sp, STK
	sti

	; call main with argc and argv
	call ___mkargv

	cli
	mov ax, [__old_ss]
	mov ss, ax
	mov ax, [__old_sp]
	mov sp, ax
	sti

	jmp far[return_point]
	jmp $

;global ___mkargv
;___mkargv: ret

global __seg, __old_seg
__seg dw 0
__old_seg dw 0
global __old_ss, __old_sp
__old_ss dw 0
__old_sp dw 0
global __ret_off, __ret_seg
__ret_off dw 0
__ret_seg dw 0

section .bss
return_point: resw 2
section .text

section .bss
STKB:
	resb 4096
STK:
