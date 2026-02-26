;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; entry.asm                        ;;
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
	MOV SP, rm_stack_top

	MOV [boot_drive], DL

	;; Ativar linha A20
	MOV AX, 0x2403
	INT 0x15
	JC a20_error
	TEST AH, AH
	JNZ a20_error

	MOV AX, 0x2402
	INT 0x15
	JC a20_error
	TEST AH, AH
	JNZ a20_error
	TEST AL, AL
	JNZ a20_ok

	MOV AX, 0x2401
	INT 0x15

	JC a20_error
	TEST AH, AH
	JNZ a20_error
a20_ok:
	LGDT [gdtr]

	MOV EAX, CR0
	OR EAX, 1
	MOV CR0, EAX

	JMP 0x08:_start32
	;; Nunca deve chegar aqui
	CLI
	HLT

a20_error:
	MOV SI, a20_error_msg
	CALL print_string
	CLI
	HLT

a20_error_msg:
	DB 'Falha ao ativar linha A20, impossivel continuar!', 0x0A, 0x0D, 0x00

print_string:
	PUSH AX
	PUSH SI
	MOV AH, 0x0E
.loop:
	LODSB
	TEST AL, AL
	JZ .end
	INT 0x10
	JMP .loop
.end:
	POP SI
	POP AX
	RET

GLOBAL boot_drive
boot_drive: DB 0

;; Cria uma entrada da GDT
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

;; Volta para o modo real
%macro real_mode 0
	CLI
	JMP WORD 0x18:%%entry16
%%entry16:
	BITS 16
	MOV AX, 0x20
	MOV DS, AX
	MOV ES, AX
	MOV FS, AX
	MOV GS, AX
	MOV SS, AX

	MOV EAX, CR0
	AND EAX, ~1 ;; Desabilitar o bit PE
	MOV CR0, EAX

	JMP WORD 0x00:%%realmode
%%realmode:
	XOR AX, AX
	MOV DS, AX
	MOV ES, AX
	MOV FS, AX
	MOV GS, AX
	MOV SS, AX
%endmacro

;; Volta pro modo protegido
%macro protected_mode 0
	CLI
	MOV EAX, CR0
	OR EAX, 1
	MOV CR0, EAX

	JMP DWORD 0x08:%%protectedmode
%%protectedmode:
	BITS 32
	MOV AX, 0x10
	MOV DS, AX
	MOV ES, AX
	MOV FS, AX
	MOV GS, AX
	MOV SS, AX
%endmacro

struc Regs
	.eax RESD 1
	.ebx RESD 1
	.ecx RESD 1
	.edx RESD 1
	.ebp RESD 1
	.esi RESD 1
	.edi RESD 1
	.ds RESW 1
	.es RESW 1
	.flags RESW 1
endstruc

rm_stack_bottom:
	TIMES 1024 DB 0
rm_stack_top:

BITS 32
;; Usa uma interrupção no modo de 16 bits
;; typedef struct Regs { uint32_t eax, ebx, ecx, edx, ebp, esi, edi; uint16_t ds, es, flags; } __attribute__((packed)) Regs;
;; void intx(unsigned char intnum, Regs *r);
GLOBAL intx
intx:
	PUSH EBP
	MOV EBP, ESP

	MOV AL, [EBP+8]
	MOV [.int+1], AL

	MOV ESI, [EBP+12]
	MOV [.struct], ESI

	PUSH DS
	PUSH ES
	PUSH FS
	PUSH GS
	PUSHAD
	PUSHFD
	MOV [.esp], ESP

	MOV ESP, rm_stack_top
	PUSH DWORD [ESI+Regs.eax]
	PUSH DWORD [ESI+Regs.ebx]
	PUSH DWORD [ESI+Regs.ecx]
	PUSH DWORD [ESI+Regs.edx]
	PUSH DWORD [ESI+Regs.ebp]
	PUSH DWORD [ESI+Regs.esi]
	PUSH DWORD [ESI+Regs.edi]
	PUSH WORD [ESI+Regs.ds]
	PUSH WORD [ESI+Regs.es]
	PUSH WORD [ESI+Regs.flags]

	real_mode

	POP WORD AX
	POP WORD ES
	POP WORD DS
	POP DWORD EDI
	POP DWORD ESI
	POP DWORD EBP
	POP DWORD EDX
	POP DWORD ECX
	POP DWORD EBX
	POP DWORD EAX
.int:
	INT 0x00
	PUSHF
	PUSH WORD ES
	PUSH WORD DS
	PUSH DWORD EDI
	PUSH DWORD ESI
	PUSH DWORD EBP
	PUSH DWORD EDX
	PUSH DWORD ECX
	PUSH DWORD EBX
	PUSH DWORD EAX

	protected_mode

	MOV ESI, [.struct]

	POP DWORD [ESI+Regs.eax]
	POP DWORD [ESI+Regs.ebx]
	POP DWORD [ESI+Regs.ecx]
	POP DWORD [ESI+Regs.edx]
	POP DWORD [ESI+Regs.ebp]
	POP DWORD [ESI+Regs.esi]
	POP DWORD [ESI+Regs.edi]
	POP WORD [ESI+Regs.ds]
	POP WORD [ESI+Regs.es]
	POP WORD [ESI+Regs.flags]

	MOV ESP, [.esp]
	POPFD
	POPAD
	POP GS
	POP FS
	POP ES
	POP DS
	POP EBP
	RET
.esp: DD 0
.struct: DD 0

SECTION .text
BITS 32
EXTERN main

EXTERN __bss_start
EXTERN __bss_end

GLOBAL _start32
_start32:
	MOV AX, 0x10
	MOV DS, AX
	MOV ES, AX
	MOV FS, AX
	MOV GS, AX
	MOV SS, AX
	MOV ESP, stack_top

	CLD

	;; zerar BSS
	XOR EAX, EAX
	MOV ECX, __bss_end
	MOV EDI, __bss_start
	SUB ECX, EDI
	REP STOSB

	MOV ECX, stack_top
	MOV EDI, stack_bottom
	SUB ECX, EDI
	REP STOSB

	CALL main

hang:
	JMP hang

SECTION	.bss 
stack_bottom: 
	RESB 4096
stack_top: