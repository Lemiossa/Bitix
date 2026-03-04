;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; entry.asm                        ;;
;; Criado por Matheus Leme Da Silva ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
BITS 32
SECTION .text
EXTERN kernel_main

EXTERN __bss_start
EXTERN __bss_end

GLOBAL _start
_start:
	CLI
	MOV ESP, stack_top
	MOV ESI, EAX

	CLD
	XOR EAX, EAX
	MOV ECX, __bss_end
	MOV EDI, __bss_start
	SUB ECX, EDI
	REP STOSB

	PUSH ESI ;; Ponteiro para boot_info
	CALL kernel_main
	ADD ESP, 4

	CLI
	HLT

SECTION .bss
stack_bottom:
	RESB 65536
stack_top:

