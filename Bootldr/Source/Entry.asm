;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Entry.asm                        ;;
;; Criado por Matheus Leme Da Silva ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
BITS 16
SECTION .entry

GLOBAL _start
_start:
	CLI
	XOR AX, AX
	MOV DS, AX
	MOV ES, AX
	MOV SS, AX
	MOV SP, 0x7C00

	;; Ativar linha A20
	IN AL, 0x92
	OR AL, 2
	OUT 0x92, AL

	LGDT [gdtr]

	MOV EAX, CR0
	OR EAX, 1
	MOV CR0, EAX

	JMP 0x08:_start32

	CLI
	HLT

%macro gdt_entry 4 ;; 1 = base, 2 = limite, 3 = access, 4 = flags
	DW (%2 & 0xFFFF) ;; Parte baixa do limite
	DW (%1 & 0xFFFF) ;; Parte baixa da base
	DB ((%1 >> 16) & 0xFF) ;; Parte do meio da base
	DB (%3 & 0xFF) ;; Access
	DB ((%4 & 0x0F) << 4) | ((%2 >> 16) & 0x0F) ;; Flags e parte alta do limite
	DB ((%1 >> 24) & 0xFF) ;; Parte alta da base
%endmacro

gdt:
	gdt_entry 0, 0x00000, 0b00000000, 0b0000 ;; NULL   | 0x00
	gdt_entry 0, 0xFFFFF, 0b10011010, 0b1100 ;; CODE32 | 0x08
	gdt_entry 0, 0xFFFFF, 0b10010010, 0b1100 ;; DATA32 | 0x10
	gdt_entry 0, 0xFFFFF, 0b10011010, 0b0000 ;; CODE16 | 0x18
	gdt_entry 0, 0xFFFFF, 0b10010010, 0b0000 ;; DATA16 | 0x20
gdtend:
gdtr:
	DW gdtend - gdt - 1
	DD gdt

SECTION .text
BITS 32
EXTERN main

GLOBAL _start32
_start32:
	MOV AX, 0x10
	MOV DS, AX
	MOV ES, AX
	MOV FS, AX
	MOV GS, AX
	MOV SS, AX
	MOV ESP, 0x7C00

	CALL main

	CLI
	HLT
