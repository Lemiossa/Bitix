;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Bootsect.asm                     ;;
;; Criado por Matheus Leme Da Silva ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
BITS 16
SECTION .text
ORG 0x7C00

addr: EQU 0x8000
start_lba: EQU 1
count: EQU 62

JMP SHORT _start
NOP
TIMES 62 - ($-$$) DB 0

;; Func principal
GLOBAL _start
_start:
	CLI
	XOR AX, AX
	MOV DS, AX
	MOV ES, AX
	MOV SS, AX
	MOV SP, 0x7C00
	STI

	;; Pegar parametros do disco
	MOV BYTE [drive], DL
	XOR DI, DI
	MOV AH, 0x08
	INT 0x13
	JC error

	INC DH
	AND CL, 0x3F

	MOV BYTE [heads], DH
	MOV BYTE [sectors_per_track], CL

	;; Ler o texto de testes
	MOV BX, (addr >> 4)
	MOV ES, BX
	MOV BX, (addr & 0x0F)

	MOV AX, (start_lba & 0xFFFF)
	MOV DX, ((start_lba >> 16) & 0xFFFF)

	MOV CX, count
load_loop:
	CMP CX, 0
	JE jump

	CMP BX, 0xFE00
	JB no_increment_segment

	SUB BX, 0x7E00
	PUSH AX
	MOV AX, ES
	ADD AX, 0x7E0
	MOV ES, AX
	POP AX
no_increment_segment:
	CALL read_sector

	ADD BX, 512

	INC AX
	ADC DX, 0

	PUSH AX
	MOV AH, 0x0E
	MOV AL, '.'
	INT 0x10
	POP AX

	DEC CX
	JMP load_loop
jump:
	MOV AH, 0x0E
	MOV AL, 'O'
	INT 0x10
	MOV AL, 'K'
	INT 0x10

	MOV DL, [drive]
	PUSH (addr >> 4)
	PUSH (addr & 0x0F)
	RETF

	CLI
	HLT

;; Imprime uma mensagem de erro e trava
error:
	MOV AH, 0x0E
	MOV AL, 'E'
	INT 0x10
	MOV AL, 'R'
	INT 0x10
	MOV AL, 'R'
	INT 0x10

	CLI
	HLT

;; Le um setor DX:AX em ES:BX
read_sector:
	PUSH DX
	PUSH AX
	PUSH CX

	;; Converter LBA para CHS
	;; setor = (lba % sectors_per_track) + 1
	;; cabeca = (lba / sectors_per_track) % heads
	;; cilindro = (lba / sectors_per_track) / heads

	DIV BYTE [sectors_per_track]
	;; AL = LBA / sectors_per_track
	;; AH = LBA % sectors_per_track
	INC AH
	MOV CL, AH ;; Sector

	XOR AH, AH
	DIV BYTE [heads]
	;; AL = (lba / sectors_per_track) / heads
	;; AH = (lba / sectors_per_track) % heads
	MOV CH, AL
	MOV DH, AH


	MOV DL, BYTE [drive]
	MOV AX, 0x0201
	INT 0x13
	JC error

	POP CX
	POP AX
	POP DX
	RET

drive: DB 0x00

heads: DB 0x00
sectors_per_track: DB 0x00

TIMES 510 - ($-$$) DB 0
DW 0x55AA

