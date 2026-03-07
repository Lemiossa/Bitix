/************************************
 * main.c                           *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <fpu.h>
#include <cpuid.h>
#include <asm.h>

#include <graphics.h>
#include <vga.h>
#include <terminal.h>
#include <io.h>

#include <boot.h>

#include <gdt.h>
#include <idt.h>
#include <pic.h>
#include <pit.h>

#include <pmm.h>
#include <vmm.h>

boot_info_t boot_info = {0};

typedef struct rsdp {
	char signature[8];
	uint8_t checksum;
	char oem_id[6];
	uint8_t revision;
	uint32_t rsdt; /* Deve ser convertido para ponteiro */
} __attribute__((packed)) rsdp_t;

typedef struct sdt_header {
	char signature[4];
	uint32_t length;
	uint8_t revision;
	uint8_t checksum;
	char oem_id[6];
	char oem_table_id[8]; /* Creio que seja string */
	uint32_t oem_revision;
	uint32_t creator_id;
	uint32_t creator_revision;
} __attribute__((packed)) sdt_header_t;

typedef struct rsdt {
	sdt_header_t header;
	uint32_t ptrs[];
} __attribute__((packed)) rsdt_t;

typedef struct madt_entry {
	uint8_t type;
	uint8_t length;
} __attribute__((packed)) madt_entry_t;

typedef struct madt_local_apic {
	madt_entry_t entry;
	uint8_t processor_id;
	uint8_t apic_id;
	uint32_t flags;
} __attribute__((packed)) madt_local_apic_t;

typedef struct madt_io_apic {
	madt_entry_t entry;
	uint8_t io_apic_id;
	uint8_t res;
	uint32_t io_apic_address;
	uint32_t gsi_base;
} __attribute__((packed)) madt_io_apic_t;

typedef struct madt {
	sdt_header_t header;
	uint32_t local_apic_address;
	uint32_t flags;
} __attribute__((packed)) madt_t;

/* Procura RSDP */
rsdp_t *acpi_find_rsdp(void *start, void *end)
{
	if (start == end || ((uint32_t)end - (uint32_t)start) < 16)
		return NULL;

	char *s = (char *)start;
	size_t size = (size_t)end - (size_t)start;

	for (size_t i = 0; i < size; i += 16)
	{
		/* Preguiça de fazer memcmp */
		if (s[i] == 'R' &&
				s[i+1] == 'S' &&
				s[i+2] == 'D' &&
				s[i+3] == ' ' &&
				s[i+4] == 'P' &&
				s[i+5] == 'T' &&
				s[i+6] == 'R' &&
				s[i+7] == ' ') {
			return (rsdp_t *)&s[i];
		}
	}

	return NULL; /* Não achou */
}

/* Retorna o ponteiro de uma entrada no RSDT com um nome */
uint32_t acpi_find_rsdt_entry(uint32_t rsdt_phys, char *name)
{
	if (!name)
		return 0;

	uint32_t rsdt_virt_addr = vmm_get_free_virt();
	if (!vmm_map(rsdt_phys, rsdt_virt_addr, PAGE_WRITE | PAGE_PRESENT))
		return 0;
	rsdt_t *rsdt = (rsdt_t *)(rsdt_virt_addr + (rsdt_phys & 0xFFF));

	uint32_t rsdt_ptr_count = (rsdt->header.length - sizeof(sdt_header_t)) / 4;

	uint32_t addr = 0;
	for (uint32_t i = 0; i < rsdt_ptr_count; i++) {
		uint32_t ptr = rsdt->ptrs[i];

		uint32_t virt = vmm_get_free_virt();

		if (!vmm_map(ptr, virt, PAGE_WRITE | PAGE_PRESENT))
			continue;

		sdt_header_t *h = (sdt_header_t *)(virt + (ptr & 0xFFF));

		if (h->signature[0] == name[0] &&
				h->signature[1] == name[1] &&
				h->signature[2] == name[2] &&
				h->signature[3] == name[3]) {
			addr = ptr;
		}

		vmm_unmap(virt);
	}

	vmm_unmap(rsdt_virt_addr);

	return addr;
}

/* Valida uma RSDP */
/* Retorna true se ela for válida */
bool acpi_check_rsdp(rsdp_t *rsdp)
{
	uint8_t *p = (uint8_t *)rsdp;
	uint8_t sum = 0;

	for (int i = 0; i < 20; i++) {
		sum += p[i];
	}

	return sum == 0;
}

/* Func principal */
void kernel_main(boot_info_t *bi)
{
	boot_info = *bi;
	gdt_init();
	idt_init();
	pic_remap(0x20, 0x28);
	if (!pmm_init())
		goto halt_no_msg;
	if (!vmm_init())
		goto halt_no_msg;

	graphics_init();
	terminal_init();
	terminal_clear(TERMINAL_DEFAULT_FG_COLOR, TERMINAL_DEFAULT_BG_COLOR);

	if (!cpuid_is_available()) {
		printf("ERRO: CPUID nao esta disponivel\r\n");
		goto halt;
	}
	cpuid_get_features();

	if (!fpu_init()) {
		printf("Falha ao inicializar FPU(80387)\r\n");
		goto halt;
	}

	printf("\033[32mKernel iniciado!\033[0m\r\n");
	printf("Modo de video: \033[32m%d\033[0mx\033[33m%d\033[0mx\033[34m%d\033[0m\r\n", boot_info.graphics.width, boot_info.graphics.height,
			boot_info.graphics.bpp);

	printf("Ponteiro para e820_table: 0x%08X\r\n", boot_info.e820_table);
	for (int count = 0; count < boot_info.e820_entry_count; count++) {
		e820_entry_t entry = boot_info.e820_table[count];
		printf("E820[%d]: 0x%08X-0x%08X:%d\r\n",
				count, (uint32_t)entry.base, (uint32_t)entry.length, entry.type);
	}

	printf("Fornecedor de CPU: %s\r\n", (char *)cpu_vendor);

	rsdp_t *rsdp = acpi_find_rsdp((void *)0xE0000, (void *)0xFFFFF);
	if (!rsdp) {
failed_to_find_rsdp:
		printf("Falha ao procurar RSDP\r\n");
		goto halt;
	}

check_rsdp:
	printf("Encontrado RSDP em 0x%08X\r\n", (uint32_t)rsdp);
	if (!acpi_check_rsdp(rsdp)) {
		printf("RSDP invalido!\r\n");
		printf("Tentando mais uma vez...\r\n");
		rsdp = acpi_find_rsdp((uint8_t *)rsdp + sizeof(rsdp_t), (void *)0xFFFFF);
		if (!rsdp)
			goto failed_to_find_rsdp;
		goto check_rsdp;
	}

	uint32_t madt_phys = acpi_find_rsdt_entry(rsdp->rsdt, "APIC");

	uint32_t madt_virt = vmm_get_free_virt();
	if (!madt_virt || !vmm_map(madt_phys, madt_virt, PAGE_WRITE | PAGE_PRESENT))
		goto halt;

	madt_t *madt = (madt_t *)(madt_virt + (madt_phys & 0xFFF));

	/* Mapear todas as paginas necessarias */
	uint32_t madt_size = madt->header.length;
	for (uint32_t i = 0; i < madt_size; i += PAGE_SIZE)
		vmm_map(madt_phys + i, madt_virt + i, PAGE_WRITE | PAGE_PRESENT);

	uint8_t *ptr = (uint8_t *)madt + sizeof(madt_t);
	uint8_t *end = (uint8_t *)madt + madt_size;

	while (ptr < end) {
		madt_entry_t *entry = (madt_entry_t *)ptr;

		switch (entry->type) {
			case 0: {
				madt_local_apic_t *lapic = (madt_local_apic_t *)entry;
				printf("lapic: apic_id: %hhu, processor_id: %hhu\r\n", lapic->apic_id, lapic->processor_id);
			} break;
			case 1: {
				madt_io_apic_t *ioapic = (madt_io_apic_t *)entry;
				printf("ioapic: io_apic_id: %hhu, io_apic_address: 0x%08X\r\n", ioapic->io_apic_id, ioapic->io_apic_address);
			} break;
		}

		ptr += entry->length;
	}

	for (uint32_t i = 0; i < madt_size; i++)
		vmm_unmap(madt_virt + i);

	vmm_unmap(madt_virt);

halt:
	printf("Sistema travado. Por favor, reinicie\r\n");
halt_no_msg:
	while (1);
}
