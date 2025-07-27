/**
 * pci.c
 * Created by Matheus Leme Da Silva
 */
#include "types.h"
#include "ports.h"
#include "util.h"
#include "pci.h"

uint32_t pci_read_config(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) 
{
    uint32_t address;
    uint32_t lbus=(uint32_t)bus;
    uint32_t lslot=(uint32_t)slot;
    uint32_t lfunc=(uint32_t)func;

    address=(uint32_t)((lbus<<16)|(lslot<<11)|
              (lfunc<<8)|(offset&0xfc)|((uint32_t)0x80000000));

    outl(PCI_CONFIG_ADDRESS, address);

    return inl(PCI_CONFIG_DATA);
}

void pci_write_config(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t value)
{
    uint32_t address;
    uint32_t lbus=(uint32_t)bus;
    uint32_t lslot=(uint32_t)slot;
    uint32_t lfunc=(uint32_t)func;

    address=(uint32_t)((lbus<<16)|(lslot<<11)|
              (lfunc<<8)|(offset&0xfc)|((uint32_t)0x80000000));

    outl(PCI_CONFIG_ADDRESS, address);
    outl(PCI_CONFIG_DATA, value);
}
