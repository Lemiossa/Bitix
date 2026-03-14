/************************************
 * cpuid.c                          *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#include <asm.h>
#include <cpuid.h>
#include <stddef.h>
#include <stdint.h>
#include <panic.h>
#include <terminal.h>
#include <debug.h>

char cpu_vendor[13] = {0};
uint32_t cpu_features_ebx = 0;
uint32_t cpu_features_ecx = 0;
uint32_t cpu_features_edx = 0;

/* Pega todas as features da CPU e coloca na variavel global CPU features ebx,
 * ecx e edx*/
void cpuid_get_features(void)
{
	if (!cpuid_is_available())
		return;

	uint32_t ebx = 0, ecx = 0, edx = 0;
	cpuid(0, NULL, &ebx, &ecx, &edx);

	((uint32_t *)cpu_vendor)[0] = ebx;
	((uint32_t *)cpu_vendor)[1] = edx;
	((uint32_t *)cpu_vendor)[2] = ecx;
	cpuid(1, NULL, &cpu_features_ebx, &cpu_features_ecx, &cpu_features_edx);
}

/* Inicializa CPUID */
/* Basicamente, usa CPUID para pegar infos */
void cpuid_init(void)
{
	debugf("CPUID: Inicializando...\r\n");
	if (!cpuid_is_available())
		panic("CPUID nao esta disponivel\r\n");

	cpuid_get_features();

	debugf("CPUID: Fornecedor de CPU: %s\r\n", (char *)cpu_vendor);
}
