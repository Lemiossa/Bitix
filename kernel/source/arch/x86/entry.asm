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
	MOV ESP, 0x90000

	PUSH EAX ;; Ponteiro para boot_info
	CALL kernel_main
	ADD ESP, 4

	CLI
	HLT

SECTION .bss
stack_bottom:
	RESB 65536
stack_top:
