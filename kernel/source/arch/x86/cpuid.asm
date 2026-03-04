;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; cpuid.asm                        ;;
;; Criado por Matheus Leme Da Silva ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
BITS 32
SECTION .text

;; https://wiki.osdev.org/CPUID
;; Retorna true se CPUID estiver disponivel
;; int cpuid_is_available(void);
GLOBAL cpuid_is_available
cpuid_is_available:
	PUSHFD ;; EFLAGS originais
	PUSHFD ;; EFLAGS que vão ser alteradas
	XOR DWORD [ESP], 0x200000 ;; Inverter bit de CPUID
	POPFD ;; Carregar as EFLAGS modificadas
	PUSHFD ;; Jogar pra CPU
	POP EAX ;; EFLAGS modificadas tambem pelo hardware
	XOR EAX, [ESP] ;; EAX Vai ter todos os BITS alterados
	POPFD ;; Carregar EFLAGS originais
	AND EAX, 0x200000 ;; Deixa somente o bit de CPUID
	;; Se o bit de CPU id estiver ativo, CPUID é suportado
	RET
