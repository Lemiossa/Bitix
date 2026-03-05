/************************************
 * fpu.c                            *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#include <stdint.h>
#include <stdbool.h>

#include <cpuid.h>
#include <asm.h>

/* Inicializa FPU */
/* Retorna false se houver erro */
bool fpu_init(void)
{
	if (!cpu_features_edx)
		cpuid_get_features();

	if (!(cpu_features_edx & CPUID_FEAT_EDX_FPU))
		return false;

	uint32_t cr0 = get_cr0();
	if (!(cr0 & CR0_ET))
		return false;

	cr0 &= ~(CR0_EM | CR0_TS); /* Desativar emulação */
	cr0 |= CR0_MP | CR0_NE;
	set_cr0(cr0);
	fninit();
	fnclex();

	uint16_t cw = fnstcw();

	cw &= ~(1 << 2);

	fldcw(cw); /* Ativar a exceção de divide by zero */

	return true;
}

