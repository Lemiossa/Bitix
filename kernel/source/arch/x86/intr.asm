;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; intr.asm                         ;;
;; Criado por Matheus Leme Da Silva ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
BITS 32
SECTION .text

%macro ISR_NO_ERR 1
GLOBAL isr_%1
isr_%1:
	PUSH DWORD 0
	PUSH DWORD %1
	JMP isr_common
%endmacro

%macro ISR_ERR 1
GLOBAL isr_%1
isr_%1:
	;; A CPU já empurra o erro aqui
	;; PUSH DWORD 0
	PUSH DWORD %1
	JMP isr_common
%endmacro

EXTERN intr_handler
GLOBAL isr_common
isr_common:
	PUSHAD

	PUSH GS
	PUSH FS
	PUSH ES
	PUSH DS

	;; Setar DS, ES, FS e GS do kernel
	MOV EAX, 0x10
	MOV DS, AX
	MOV ES, AX
	MOV GS, AX
	MOV FS, AX

	PUSH ESP
	CALL intr_handler
	ADD ESP, 4 ;; Remover o parametro passado

	POP DS
	POP ES
	POP FS
	POP GS

	POPAD

	ADD ESP, 8 ;; Remover int_no e err_code
	IRETD


;; Troca o contexto usando intr_frame
;; void switch_context(intr_frame_t *f);
GLOBAL switch_context
switch_context:
	MOV ESP, [ESP+4]

	POP DS
	POP ES
	POP FS
	POP GS

	POPAD

	ADD ESP, 8 ;; Remover int_no e err_code
	IRETD

ISR_NO_ERR 0
ISR_NO_ERR 1
ISR_NO_ERR 2
ISR_NO_ERR 3
ISR_NO_ERR 4
ISR_NO_ERR 5
ISR_NO_ERR 6
ISR_NO_ERR 7
ISR_ERR 8
ISR_NO_ERR 9
ISR_ERR 10
ISR_ERR 11
ISR_ERR 12
ISR_ERR 13
ISR_ERR 14
ISR_NO_ERR 15
ISR_NO_ERR 16
ISR_ERR 17
ISR_NO_ERR 18
ISR_NO_ERR 19
ISR_NO_ERR 20
ISR_ERR 21
ISR_NO_ERR 22
ISR_NO_ERR 23
ISR_NO_ERR 24
ISR_NO_ERR 25
ISR_NO_ERR 26
ISR_NO_ERR 27
ISR_NO_ERR 28
ISR_NO_ERR 29
ISR_NO_ERR 30
ISR_NO_ERR 31

%assign i 32
%rep 224
	ISR_NO_ERR i
	%assign i i+1
%endrep

GLOBAL isrs
isrs:
%assign i 0
%rep 256
	DD isr_%+i
	%assign i i+1
%endrep
