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

/* FNINIT */
static inline void fninit(void)
{
	__asm__ volatile ("FNINIT");
}

/* FNCLEX */
static inline void fnclex(void)
{
	__asm__ volatile ("FNCLEX");
}


/* FWAIT */
static inline void fwait(void)
{
	__asm__ volatile ("FWAIT");
}

/* FNSTCW */
static inline uint16_t fnstcw(void)
{
	uint16_t cw = 0;
	__asm__ volatile ("FNSTCW %0":"=m"(cw));
	return cw;
}

/* FLDCW */
static inline void fldcw(uint16_t cw)
{
	__asm__ volatile ("FLDCW %0" ::"m"(cw));
}


/* FNSTSW */
static inline uint16_t fnstsw(void)
{
	uint16_t sw = 0;
	__asm__ volatile ("FNSTSW %0":"=m"(sw));
	return sw;
}

/* FLDSW */
static inline void fldsw(uint16_t sw)
{
	__asm__ volatile ("FLDCW %0" ::"m"(sw));
}

#endif /* ASM_H */
