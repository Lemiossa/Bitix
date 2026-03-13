/************************************
 * cpuid.h                          *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#ifndef CPUID_H
#define CPUID_H
#include <stdint.h>

extern char cpu_vendor[13];
extern uint32_t cpu_features_ebx;
extern uint32_t cpu_features_ecx;
extern uint32_t cpu_features_edx;

enum
{
	CPUID_BRAND_INDEX = 0xFF << 0,
	CPUID_CLFLUSH_LINE_SIZE = 0xFF << 8,
	CPUID_APIC_ID_SPACE = 0xFF << 16,
	CPUID_INITIAL_APIC_ID = 0xFF << 24,

	CPUID_SSE3 = 1 << 0,
	CPUID_PCLMUL = 1 << 1,
	CPUID_DTES64 = 1 << 2,
	CPUID_MONITOR = 1 << 3,
	CPUID_DS_CPL = 1 << 4,
	CPUID_VMX = 1 << 5,
	CPUID_SMX = 1 << 6,
	CPUID_EST = 1 << 7,
	CPUID_TM2 = 1 << 8,
	CPUID_SSSE3 = 1 << 9,
	CPUID_CID = 1 << 10,
	CPUID_SDBG = 1 << 11,
	CPUID_FMA = 1 << 12,
	CPUID_CX16 = 1 << 13,
	CPUID_XTPR = 1 << 14,
	CPUID_PDCM = 1 << 15,
	CPUID_PCID = 1 << 17,
	CPUID_DCA = 1 << 18,
	CPUID_SSE4_1 = 1 << 19,
	CPUID_SSE4_2 = 1 << 20,
	CPUID_X2APIC = 1 << 21,
	CPUID_MOVBE = 1 << 22,
	CPUID_POPCNT = 1 << 23,
	CPUID_TSC = 1 << 24,
	CPUID_AES = 1 << 25,
	CPUID_XSAVE = 1 << 26,
	CPUID_OSXSAVE = 1 << 27,
	CPUID_AVX = 1 << 28,
	CPUID_F16C = 1 << 29,
	CPUID_RDRAND = 1 << 30,
	CPUID_HYPERVISOR = 1 << 31,

	CPUID_FPU = 1 << 0,
	CPUID_VME = 1 << 1,
	CPUID_DE = 1 << 2,
	CPUID_PSE = 1 << 3,
	CPUID_TSC2 = 1 << 4,
	CPUID_MSR = 1 << 5,
	CPUID_PAE = 1 << 6,
	CPUID_MCE = 1 << 7,
	CPUID_CX8 = 1 << 8,
	CPUID_APIC = 1 << 9,
	CPUID_SEP = 1 << 11,
	CPUID_MTRR = 1 << 12,
	CPUID_PGE = 1 << 13,
	CPUID_MCA = 1 << 14,
	CPUID_CMOV = 1 << 15,
	CPUID_PAT = 1 << 16,
	CPUID_PSE36 = 1 << 17,
	CPUID_PSN = 1 << 18,
	CPUID_CLFLUSH = 1 << 19,
	CPUID_DS = 1 << 21,
	CPUID_ACPI = 1 << 22,
	CPUID_MMX = 1 << 23,
	CPUID_FXSR = 1 << 24,
	CPUID_SSE = 1 << 25,
	CPUID_SSE2 = 1 << 26,
	CPUID_SS = 1 << 27,
	CPUID_HTT = 1 << 28,
	CPUID_TM = 1 << 29,
	CPUID_IA64 = 1 << 30,
	CPUID_PBE = 1 << 31
};

int cpuid_is_available(void);
void cpuid_get_features(void);
void cpuid_init(void);

#endif /* CPUID_H */
