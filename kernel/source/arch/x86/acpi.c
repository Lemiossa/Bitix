/************************************
 * acpi.c                           *
 * Criado por Matheus Leme Da Silva *
 ***********************************/
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <panic.h>
#include <pmm.h>
#include <vmm.h>
#include <acpi.h>
#include <terminal.h>
#include <debug.h>

typedef struct rsdp
{
	char signature[8];
	uint8_t checksum;
	char oem_id[6];
	uint8_t revision;
	uint32_t rsdt; /* Deve ser convertido para ponteiro */
} __attribute__((packed)) rsdp_t;

typedef struct sdt_header
{
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

typedef struct rsdt
{
	sdt_header_t header;
	uint32_t ptrs[];
} __attribute__((packed)) rsdt_t;

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
		if (s[i] == 'R' && s[i + 1] == 'S' && s[i + 2] == 'D' &&
			s[i + 3] == ' ' && s[i + 4] == 'P' && s[i + 5] == 'T' &&
			s[i + 6] == 'R' && s[i + 7] == ' ')
		{
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
	for (uint32_t i = 0; i < rsdt_ptr_count; i++)
	{
		uint32_t ptr = rsdt->ptrs[i];

		uint32_t virt = vmm_get_free_virt();

		if (!vmm_map(ptr, virt, PAGE_WRITE | PAGE_PRESENT))
			continue;

		sdt_header_t *h = (sdt_header_t *)(virt + (ptr & 0xFFF));

		if (h->signature[0] == name[0] && h->signature[1] == name[1] &&
			h->signature[2] == name[2] && h->signature[3] == name[3])
		{
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

	for (int i = 0; i < 20; i++)
	{
		sum += p[i];
	}

	return sum == 0;
}

/* Inicializa ACPI */
void acpi_init(void)
{
	debugf("ACPI: Inicializando...\r\n");
	debugf("ACPI: Procurando RSDP...\r\n");
	rsdp_t *rsdp = acpi_find_rsdp((void *)0xE0000, (void *)0xFFFFF);
	if (!rsdp)
	{
	failed_to_find_rsdp:
		panic("ACPI: Falha ao procurar RSDP\r\n");
	}

check_rsdp:
	if (!acpi_check_rsdp(rsdp))
	{
		rsdp =
			acpi_find_rsdp((uint8_t *)rsdp + sizeof(rsdp_t), (void *)0xFFFFF);
		if (!rsdp)
			goto failed_to_find_rsdp;
		goto check_rsdp;
	}

	debugf("ACPI: Encontrado RSDP em 0x%08X\r\n", (uint32_t)rsdp);
}
