/************************************
 * asm.h                            *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#ifndef ASM_H
#define ASM_H
#include <stdint.h>

#define CR0_PE (1 << 0)
#define CR0_MP (1 << 1)
#define CR0_EM (1 << 2)
#define CR0_TS (1 << 3)
#define CR0_ET (1 << 4)
#define CR0_NE (1 << 5)
#define CR0_WP (1 << 16)
#define CR0_AM (1 << 18)
#define CR0_NW (1 << 29)
#define CR0_CD (1 << 30)
#define CR0_PG (1 << 31)

#define CR3_PWT (1 << 3)
#define CR3_PCD (1 << 4)

#define CR4_VME        (1 << 0)
#define CR4_PVI        (1 << 1)
#define CR4_TSD        (1 << 2)
#define CR4_DE         (1 << 3)
#define CR4_PSE        (1 << 4)
#define CR4_PAE        (1 << 5)
#define CR4_MCE        (1 << 6)
#define CR4_PGE        (1 << 7)
#define CR4_PCE        (1 << 8)
#define CR4_OSFXSR     (1 << 9)
#define CR4_OSXMMEXCPT (1 << 10)
#define CR4_UMIP       (1 << 11)
#define CR4_LA57       (1 << 12)
#define CR4_VMXE       (1 << 13)
#define CR4_SMXE       (1 << 14)
#define CR4_FSGSBASE   (1 << 16)
#define CR4_PCIDE      (1 << 17)
#define CR4_OSXSAVE    (1 << 18)
#define CR4_SMEP       (1 << 20)
#define CR4_SMAP       (1 << 21)
#define CR4_PKE        (1 << 22)

/* Seta o CR0 */
static inline void set_cr0(uint32_t v)
{
	__asm__ volatile("MOV %0, %%CR0" ::"r"(v):"memory");
}

/* Retorna o valor em cr0 */
static inline uint32_t get_cr0(void)
{
	uint32_t v = 0;
	__asm__ volatile("MOV %%CR0, %0" :"=r"(v)::"memory");
	return v;
}

/* Retorna o valor em cr2 */
static inline uint32_t get_cr2(void)
{
	uint32_t v = 0;
	__asm__ volatile("MOV %%CR2, %0" :"=r"(v)::"memory");
	return v;
}

/* Seta o CR3 */
static inline void set_cr3(uint32_t v)
{
	__asm__ volatile("MOV %0, %%CR3" ::"r"(v):"memory");
}

/* Retorna o valor em cr3 */
static inline uint32_t get_cr3(void)
{
	uint32_t v = 0;
	__asm__ volatile("MOV %%CR3, %0" :"=r"(v)::"memory");
	return v;
}

/* Seta o CR4 */
static inline void set_cr4(uint32_t v)
{
	__asm__ volatile("MOV %0, %%CR4" ::"r"(v):"memory");
}

/* Retorna o valor em cr4 */
static inline uint32_t get_cr4(void)
{
	uint32_t v = 0;
	__asm__ volatile("MOV %%CR4, %0" :"=r"(v)::"memory");
	return v;
}

/* https://wiki.osdev.org/CPUID */
int cpuid_is_available(void);
/* Executa CPUID */
static inline void cpuid(uint32_t code, uint32_t *a, uint32_t *b, uint32_t *c, uint32_t *d)
{
	uint32_t eax = code, ebx = b?*b:0, ecx = c?*c:0, edx = d?*d:0;
	__asm__ volatile (
			"CPUID"
			: "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
			: "a"(eax)
			);

	if (a)
		*a = eax;

	if (b)
		*b = ebx;

	if (c)
		*c = ecx;

	if (d)
		*d = edx;
}

enum {
	CPUID_FEAT_EBX_BRAND_INDEX        = 0xFF << 0,
	CPUID_FEAT_EBX_CLFLUSH_LINE_SIZE  = 0xFF << 8,
	CPUID_FEAT_EBX_APIC_ID_SPACE      = 0xFF << 16,
	CPUID_FEAT_EBX_INITIAL_APIC_ID    = 0xFF << 24,

	CPUID_FEAT_ECX_SSE3               = 1 << 0,
	CPUID_FEAT_ECX_PCLMUL             = 1 << 1,
	CPUID_FEAT_ECX_DTES64             = 1 << 2,
	CPUID_FEAT_ECX_MONITOR            = 1 << 3,
	CPUID_FEAT_ECX_DS_CPL             = 1 << 4,
	CPUID_FEAT_ECX_VMX                = 1 << 5,
	CPUID_FEAT_ECX_SMX                = 1 << 6,
	CPUID_FEAT_ECX_EST                = 1 << 7,
	CPUID_FEAT_ECX_TM2                = 1 << 8,
	CPUID_FEAT_ECX_SSSE3              = 1 << 9,
	CPUID_FEAT_ECX_CID                = 1 << 10,
	CPUID_FEAT_ECX_SDBG               = 1 << 11,
	CPUID_FEAT_ECX_FMA                = 1 << 12,
	CPUID_FEAT_ECX_CX16               = 1 << 13,
	CPUID_FEAT_ECX_XTPR               = 1 << 14,
	CPUID_FEAT_ECX_PDCM               = 1 << 15,
	CPUID_FEAT_ECX_PCID               = 1 << 17,
	CPUID_FEAT_ECX_DCA                = 1 << 18,
	CPUID_FEAT_ECX_SSE4_1             = 1 << 19,
	CPUID_FEAT_ECX_SSE4_2             = 1 << 20,
	CPUID_FEAT_ECX_X2APIC             = 1 << 21,
	CPUID_FEAT_ECX_MOVBE              = 1 << 22,
	CPUID_FEAT_ECX_POPCNT             = 1 << 23,
	CPUID_FEAT_ECX_TSC                = 1 << 24,
	CPUID_FEAT_ECX_AES                = 1 << 25,
	CPUID_FEAT_ECX_XSAVE              = 1 << 26,
	CPUID_FEAT_ECX_OSXSAVE            = 1 << 27,
	CPUID_FEAT_ECX_AVX                = 1 << 28,
	CPUID_FEAT_ECX_F16C               = 1 << 29,
	CPUID_FEAT_ECX_RDRAND             = 1 << 30,
	CPUID_FEAT_ECX_HYPERVISOR         = 1 << 31,

	CPUID_FEAT_EDX_FPU                = 1 << 0,
	CPUID_FEAT_EDX_VME                = 1 << 1,
	CPUID_FEAT_EDX_DE                 = 1 << 2,
	CPUID_FEAT_EDX_PSE                = 1 << 3,
	CPUID_FEAT_EDX_TSC                = 1 << 4,
	CPUID_FEAT_EDX_MSR                = 1 << 5,
	CPUID_FEAT_EDX_PAE                = 1 << 6,
	CPUID_FEAT_EDX_MCE                = 1 << 7,
	CPUID_FEAT_EDX_CX8                = 1 << 8,
	CPUID_FEAT_EDX_APIC               = 1 << 9,
	CPUID_FEAT_EDX_SEP                = 1 << 11,
	CPUID_FEAT_EDX_MTRR               = 1 << 12,
	CPUID_FEAT_EDX_PGE                = 1 << 13,
	CPUID_FEAT_EDX_MCA                = 1 << 14,
	CPUID_FEAT_EDX_CMOV               = 1 << 15,
	CPUID_FEAT_EDX_PAT                = 1 << 16,
	CPUID_FEAT_EDX_PSE36              = 1 << 17,
	CPUID_FEAT_EDX_PSN                = 1 << 18,
	CPUID_FEAT_EDX_CLFLUSH            = 1 << 19,
	CPUID_FEAT_EDX_DS                 = 1 << 21,
	CPUID_FEAT_EDX_ACPI               = 1 << 22,
	CPUID_FEAT_EDX_MMX                = 1 << 23,
	CPUID_FEAT_EDX_FXSR               = 1 << 24,
	CPUID_FEAT_EDX_SSE                = 1 << 25,
	CPUID_FEAT_EDX_SSE2               = 1 << 26,
	CPUID_FEAT_EDX_SS                 = 1 << 27,
	CPUID_FEAT_EDX_HTT                = 1 << 28,
	CPUID_FEAT_EDX_TM                 = 1 << 29,
	CPUID_FEAT_EDX_IA64               = 1 << 30,
	CPUID_FEAT_EDX_PBE                = 1 << 31
};


#endif /* ASM_H */
