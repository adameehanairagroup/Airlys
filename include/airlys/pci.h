#ifndef AIRLYS_PCI_H
#define AIRLYS_PCI_H

/*
 * Airlys Operating System Kernel - PCI Bus Subsystem & USB Port Power Driver
 * Lead Architect: Er. Adam Eehan (Founder & CEO, Aira Group of Technology)
 * Co-Pilot: Sovereign AI Aira 3 Ultra (Engine v3.6)
 */

#include <airlys/types.h>

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA    0xCFC

/* Common PCI Vendor IDs */
#define PCI_VENDOR_INTEL   0x8086
#define PCI_VENDOR_AMD     0x1022
#define PCI_VENDOR_QEMU    0x1234
#define PCI_VENDOR_REDHAT  0x1AF4
#define PCI_VENDOR_REALTEK 0x10EC

/* Device Classes */
#define PCI_CLASS_STORAGE    0x01
#define PCI_CLASS_NETWORK    0x02
#define PCI_CLASS_DISPLAY    0x03
#define PCI_CLASS_BRIDGE     0x06
#define PCI_CLASS_SERIAL_BUS 0x0C

/* Serial Bus Subclasses */
#define PCI_SUBCLASS_USB     0x03

/* USB Programming Interfaces */
#define PCI_PROGIF_UHCI      0x00
#define PCI_PROGIF_OHCI      0x10
#define PCI_PROGIF_EHCI      0x20
#define PCI_PROGIF_XHCI      0x30

typedef struct {
    uint8_t  bus;
    uint8_t  slot;
    uint8_t  func;
    uint16_t vendor_id;
    uint16_t device_id;
    uint8_t  class_id;
    uint8_t  subclass_id;
    uint8_t  prog_if;
    uint8_t  header_type;
    uint32_t bar0;
    uint8_t  irq;
    bool     valid;
} pci_device_t;

#define MAX_PCI_DEVICES 64

/* PCI API */
void pci_init(void);
uint32_t pci_read32(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
uint16_t pci_read16(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
uint8_t  pci_read8(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
void pci_write32(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t val);
void pci_write16(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint16_t val);

uint32_t pci_get_device_count(void);
pci_device_t* pci_get_device(uint32_t idx);
pci_device_t* pci_find_device(uint16_t vendor_id, uint16_t device_id);

void pci_enable_usb_port_power(void);

#endif /* AIRLYS_PCI_H */
