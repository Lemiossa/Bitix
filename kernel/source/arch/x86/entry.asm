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
	MOV ESP, 0x90000
	MOV EBX, EAX

	CLD
	XOR EAX, EAX
	MOV ECX, __bss_end
	MOV EDI, __bss_start
	SUB ECX, EDI
	REP STOSB

	MOV EAX, EBX
	PUSH EAX ;; Ponteiro para boot_info
	CALL kernel_main
	ADD ESP, 4

	CLI
	HLT

SECTION .bss
stack_bottom:
	RESB 65536
stack_top:
