;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; entry.asm                        ;;
;; Criado por Matheus Leme Da Silva ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
BITS 32
SECTION .text
EXTERN kernel_main

GLOBAL _start
_start:
	CLI
	MOV ESP, stack_top

	PUSH EAX
	CALL kernel_main
	ADD ESP, 4

	CLI
	HLT

SECTION .bss
stack_bottom:
	RESB 65536
stack_top:
