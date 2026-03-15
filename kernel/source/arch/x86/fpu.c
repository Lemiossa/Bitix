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
#include <util.h>

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
}

/* Cria um novo contexto de FPU */
void fpu_create_new_context(void *out)
{
	debugf("FPU: context addr = 0x%08X\r\n", (uint32_t)out);
	uint8_t old_ctx[124] = {0};
	uint8_t *aligned = (uint8_t *)ALIGN_UP((uint32_t)old_ctx, 16);
	fnsave(aligned);
	fninit();
	fnsave(out);
	frstor(aligned);
}
