#include "io.h"
#include "print.h"
#include "pci.h"

/* temporary */
#include "ata.h"

enum {
    PCI_CONFIG_ADDRESS = 0xCF8,
    PCI_CONFIG_DATA = 0xCFC
};

/* PCI_CONFIG_ADDRESS
 * 31: enable bit
 * 30-24: reserved
 * 23-16: bus number
 * 15-11: device number
 * 10-8: function number
 * 7-2: register number
 * 1-0: zero
 */

static void pci_check_bus(uint8_t bus);


static uint16_t pci_config_read_word(uint8_t bus, uint8_t slot,
        uint8_t func, uint8_t offset)
{
    uint32_t address;
    uint32_t lbus  = (uint32_t)bus;
    uint32_t lslot = (uint32_t)slot;
    uint32_t lfunc = (uint32_t)func;

    /* create configuration address */
    address = (uint32_t)((lbus << 16) | (lslot << 11) |
            (lfunc << 8) | (offset & 0xFC) | ((uint32_t)0x80000000));

    /* write out the address */
    outportl(PCI_CONFIG_ADDRESS, address);
    /* read in the data */
    /* (offset & 2) * 8) = 0 will choose the first word of the 32 bits register */
    return (uint16_t)((inportl(PCI_CONFIG_DATA) >> ((offset & 2) * 8)) & 0xFFFF);
}

static uint8_t pci_get_header_type(uint8_t bus, uint8_t device, uint8_t function)
{
    uint16_t word = pci_config_read_word(bus, device, function, 0x0E);
    return (uint8_t)word & 0xFF;
}

static uint16_t pci_get_vendor_id(uint8_t bus, uint8_t device, uint8_t function)
{
    return pci_config_read_word(bus, device, function, 0);
}

static uint16_t pci_get_device_id(uint8_t bus, uint8_t device, uint8_t function)
{
    return pci_config_read_word(bus, device, function, 2);
}

static uint8_t pci_get_interrupt_line(uint8_t bus, uint8_t device, uint8_t function)
{
    uint16_t word = pci_config_read_word(bus, device, function, 0x3C);
    return (uint8_t)word & 0xFF;
}

static uint8_t pci_get_class(uint8_t bus, uint8_t device, uint8_t function)
{
    uint16_t word = pci_config_read_word(bus, device, function, 0x0A);
    return (word >> 8) & 0xFF;
}

static uint8_t pci_get_subclass(uint8_t bus, uint8_t device, uint8_t function)
{
    uint16_t word = pci_config_read_word(bus, device, function, 0x0A);
    return (uint8_t)word & 0xFF;
}

static uint8_t pci_get_secondary_bus(uint8_t bus, uint8_t device, uint8_t function)
{
    uint16_t word = pci_config_read_word(bus, device, function, 0x18);
    return (word >> 8) & 0xFF;
}

/* Assumes that function Header type is 0x00 (not a bridge device) */
static uint32_t pci_get_base_address(uint8_t bus, uint8_t device, uint8_t function, uint8_t barnum)
{
    KASSERT(barnum < 6);
    uint8_t offset = 0x10 + (barnum * 4);

    uint16_t low = pci_config_read_word(bus, device, function, offset);
    uint16_t high = pci_config_read_word(bus, device, function, offset + 2);

    return low & (high << 16);
}

static void pci_check_function(uint8_t bus, uint8_t device, uint8_t function)
{
    uint16_t vendor_id = pci_get_vendor_id(bus, device, function);
    uint16_t device_id = pci_get_device_id(bus, device, function);
    uint8_t class = pci_get_class(bus, device, function);
    uint8_t subclass = pci_get_subclass(bus, device, function);
    uint8_t interrupt = pci_get_interrupt_line(bus, device, function);
    DEBUGF("PCI bus %d, dev %d, func: %d, venID: %04X, devID: %04X, class: %02X, "
            "subclass: %02X, int: %02d\n", bus, device,
            function, vendor_id, device_id, class, subclass, interrupt);

    if (class == 0x01) {
        uint32_t bar0 = pci_get_base_address(bus, device, function, 0);
        uint32_t bar1 = pci_get_base_address(bus, device, function, 1);
        uint32_t bar2 = pci_get_base_address(bus, device, function, 2);
        uint32_t bar3 = pci_get_base_address(bus, device, function, 3);
        uint32_t bar4 = pci_get_base_address(bus, device, function, 4);
        ide_initialize(bar0, bar1, bar2, bar3, bar4);
    }

    /* recursive scan */
    if ((class == 0x06) && (subclass == 0x04)) {
        uint8_t secondary_bus = pci_get_secondary_bus(bus, device, function);
        pci_check_bus(secondary_bus);
    }
}

static void pci_check_device(uint8_t bus, uint8_t device)
{
    uint16_t vendor_id = pci_get_vendor_id(bus, device, 0);
    if (0xFFFF == vendor_id) {
        return;        /* Device doesn't exist */
    }

    pci_check_function(bus, device, 0);

    uint8_t header_type = pci_get_header_type(bus, device, 0);
    if ((header_type & 0x80) != 0) {
        /* It is a multi-function device, so check remaining functions */
        uint8_t function = 0;
        for (function = 1; function < 8; function++) {
            if (pci_get_vendor_id(bus, device, function) != 0xFFFF) {
                pci_check_function(bus, device, function);
            }
        }
    }
}

static void pci_check_bus(uint8_t bus)
{
    uint8_t device;

    for (device = 0; device < 32; device++) {
        pci_check_device(bus, device);
    }
}

void pci_check_all_buses(void)
{
    pci_check_bus(0);

    uint8_t header_type = pci_get_header_type(0, 0, 0);
    if ((header_type & 0x80) != 0) {
        /* Multiple PCI host controllers */
        uint8_t function;
        for (function = 1; function < 8; function++) {
            if (pci_get_vendor_id(0, 0, function) != 0xFFFF) {
                pci_check_bus(function);
            }
        }
    }
}

/** Brute-force scan of all buses instead of recursive scan */
/* void pci_check_all_buses(void) */
/* { */
/*     unsigned int bus = 0; */
/*     for (bus = 0; bus < 256; bus++) { */
/*         uint8_t device; */
/*         for (device = 0; device < 32; device++) { */
/*             pci_check_device((uint8_t)bus, device); */
/*         } */
/*     } */
/* } */

