/************************************
 * acpi.h                           *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#ifndef ACPI_H
#define ACPI_H
#include <stdbool.h>
#include <stdint.h>

#define MAX_CPUS 256

typedef struct cpu
{
	uint8_t apic_id;
	uint8_t processor_id;
	uint32_t flags;
} cpu_t;

extern cpu_t cpus[MAX_CPUS];
extern int cpu_count;

uint32_t acpi_lapic_read(uint32_t offset);
void acpi_lapic_write(uint32_t offset, uint32_t value);
void acpi_ioapic_write(uint32_t reg, uint32_t val);
uint32_t acpi_ioapic_read(uint32_t reg);
void acpi_init(void);

#endif /* ACPI_H */
