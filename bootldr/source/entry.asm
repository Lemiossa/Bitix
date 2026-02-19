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

	MOV [boot_drive], DL

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
	.ds RESD 1
	.es RESD 1
	.eflags RESD 1
endstruc

real_mode_stack:
	TIMES Regs_size DB 0
real_mode_stack_top:

;; Usa uma interrupção no modo de 16 bits(modo real)
;; typedef struct Regs { uint32_t eax, ebx, ecx, edx, ebp, esi, edi; uint16_t ds, es, flags; } __attribute__((packed)) Regs;
;; void int16(uint8_t intnum, Regs *r);
GLOBAL int16
int16:
	BITS 32
	MOV AL, [ESP+4]
	MOV [.int+1], AL

	MOV ESI, [ESP+8]
	MOV [.struct], ESI

	PUSH DS
	PUSH ES
	PUSH FS
	PUSH GS
	PUSHAD
	PUSHFD
	MOV [.esp], ESP

	MOV ESP, real_mode_stack_top
	PUSH DWORD [ESI+Regs.eax]
	PUSH DWORD [ESI+Regs.ebx]
	PUSH DWORD [ESI+Regs.ecx]
	PUSH DWORD [ESI+Regs.edx]
	PUSH DWORD [ESI+Regs.ebp]
	PUSH DWORD [ESI+Regs.esi]
	PUSH DWORD [ESI+Regs.edi]
	PUSH DWORD [ESI+Regs.ds]
	PUSH DWORD [ESI+Regs.es]
	PUSH DWORD [ESI+Regs.eflags]

	real_mode

	MOV SP, real_mode_stack

	POP DWORD EAX
	POP DWORD EAX
	MOV ES, AX
	POP DWORD EAX
	MOV DS, AX
	POP DWORD EDI
	POP DWORD ESI
	POP DWORD EBP
	POP DWORD EDX
	POP DWORD ECX
	POP DWORD EBX
	POP DWORD EAX
.int:
	INT 0x00
	PUSHFD
	PUSH DWORD ES
	PUSH DWORD DS
	PUSH DWORD EDI
	PUSH DWORD ESI
	PUSH DWORD EBP
	PUSH DWORD EDX
	PUSH DWORD ECX
	PUSH DWORD EBX
	PUSH DWORD EAX

	PUSH EAX
	protected_mode
	POP EAX

	MOV ESI, [.struct]

	POP DWORD [ESI+Regs.eax]
	POP DWORD [ESI+Regs.ebx]
	POP DWORD [ESI+Regs.ecx]
	POP DWORD [ESI+Regs.edx]
	POP DWORD [ESI+Regs.ebp]
	POP DWORD [ESI+Regs.esi]
	POP DWORD [ESI+Regs.edi]
	POP DWORD [ESI+Regs.ds]
	POP DWORD [ESI+Regs.es]
	POP DWORD [ESI+Regs.eflags]

	MOV ESP, [.esp]
	POPFD
	POPAD
	POP GS
	POP FS
	POP ES
	POP DS
	RET
.esp: DD 0
.struct: DD 0

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

hang:
	JMP hang

