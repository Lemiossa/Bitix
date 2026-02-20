;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Entry.asm                        ;;
;; Criado por Matheus Leme Da Silva ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
BITS 32
SECTION .text
EXTERN main

GLOBAL _start
_start:
	MOV AH, 0x0E
	MOV AL, 'X'
	INT 0x10

	CLI
	MOV ESP, 0x90000

	MOV BYTE [0xB8000], 'X'

	;;PUSH EAX
	;;CALL main
	;;ADD ESP, 2

	CLI
	HLT

