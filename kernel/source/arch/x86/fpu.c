/************************************
 * fpu.c                            *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#include <stdbool.h>
#include <stdint.h>
#include <panic.h>
#include <asm.h>
#include <cpuid.h>
#include <debug.h>

/* Inicializa FPU */
/* Retorna false se houver erro */
void fpu_init(void)
{
	debugf("FPU: Inicializando...\r\n");
	if (!(cpu_features_edx & CPUID_FPU))
		panic("FPU: A maquina nao possui FPU\r\n");

	uint32_t cr0 = get_cr0();
	if (!(cr0 & CR0_ET))
		panic("FPU: BIT ET em CR0 nao esta ativo\r\n");

	cr0 &= ~(CR0_EM | CR0_TS); /* Desativar emulação */
	cr0 |= CR0_MP | CR0_NE;
	set_cr0(cr0);
	fninit();
	fnclex();

	uint16_t cw = fnstcw();

	cw &= ~(1 << 2);

	fldcw(cw); /* Ativar a exceção de divide by zero */
}
