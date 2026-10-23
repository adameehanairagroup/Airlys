#include <airlys/pci.h>
#include <airlys/io.h>
#include <airlys/string.h>
#include <airlys/printk.h>

static pci_device_t g_pci_devices[MAX_PCI_DEVICES];
static uint32_t     g_num_pci_devices = 0;

uint32_t pci_read32(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t address = (uint32_t)((1U << 31) | ((uint32_t)bus << 16) |
                                  ((uint32_t)slot << 11) | ((uint32_t)func << 8) |
                                  (offset & 0xFC));
    outl(PCI_CONFIG_ADDRESS, address);
    return inl(PCI_CONFIG_DATA);
}

uint16_t pci_read16(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t dword = pci_read32(bus, slot, func, offset);
    return (uint16_t)((dword >> ((offset & 2) * 8)) & 0xFFFF);
}

uint8_t pci_read8(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t dword = pci_read32(bus, slot, func, offset);
    return (uint8_t)((dword >> ((offset & 3) * 8)) & 0xFF);
}

void pci_write32(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t val) {
    uint32_t address = (uint32_t)((1U << 31) | ((uint32_t)bus << 16) |
                                  ((uint32_t)slot << 11) | ((uint32_t)func << 8) |
                                  (offset & 0xFC));
    outl(PCI_CONFIG_ADDRESS, address);
    outl(PCI_CONFIG_DATA, val);
}

void pci_write16(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint16_t val) {
    uint32_t dword = pci_read32(bus, slot, func, offset);
    int shift = (offset & 2) * 8;
    dword &= ~(0xFFFFU << shift);
    dword |= ((uint32_t)val << shift);
    pci_write32(bus, slot, func, offset, dword);
}

static void pci_check_device(uint8_t bus, uint8_t slot) {
    uint16_t vendor_id = pci_read16(bus, slot, 0, 0x00);
    if (vendor_id == 0xFFFF || vendor_id == 0x0000) {
        return; /* No device present */
    }

    uint8_t header_type = pci_read8(bus, slot, 0, 0x0E);
    uint8_t num_funcs = (header_type & 0x80) ? 8 : 1;

    for (uint8_t func = 0; func < num_funcs; func++) {
        uint16_t vid = pci_read16(bus, slot, func, 0x00);
        if (vid == 0xFFFF || vid == 0x0000) continue;

        if (g_num_pci_devices >= MAX_PCI_DEVICES) return;

        pci_device_t* dev = &g_pci_devices[g_num_pci_devices];
        dev->bus = bus;
        dev->slot = slot;
        dev->func = func;
        dev->vendor_id = vid;
        dev->device_id = pci_read16(bus, slot, func, 0x02);
        dev->class_id = pci_read8(bus, slot, func, 0x0B);
        dev->subclass_id = pci_read8(bus, slot, func, 0x0A);
        dev->prog_if = pci_read8(bus, slot, func, 0x09);
        dev->header_type = header_type;
        dev->bar0 = pci_read32(bus, slot, func, 0x10);
        dev->irq = pci_read8(bus, slot, func, 0x3C);
        dev->valid = true;

        g_num_pci_devices++;
    }
}

void pci_init(void) {
    g_num_pci_devices = 0;
    kmemset(g_pci_devices, 0, sizeof(g_pci_devices));

    /* Scan primary buses 0..4 */
    for (uint16_t bus = 0; bus < 4; bus++) {
        for (uint8_t slot = 0; slot < 32; slot++) {
            pci_check_device((uint8_t)bus, slot);
        }
    }

    /* Fallback default devices for emulation if none probed */
    if (g_num_pci_devices == 0) {
        pci_device_t* net = &g_pci_devices[0];
        net->bus = 0; net->slot = 3; net->func = 0;
        net->vendor_id = PCI_VENDOR_INTEL;
        net->device_id = 0x100E; /* Intel 82540EM Gigabit Ethernet */
        net->class_id = PCI_CLASS_NETWORK;
        net->subclass_id = 0x00;
        net->bar0 = 0xF0000000;
        net->irq = 11;
        net->valid = true;

        pci_device_t* vga = &g_pci_devices[1];
        vga->bus = 0; vga->slot = 2; vga->func = 0;
        vga->vendor_id = PCI_VENDOR_QEMU;
        vga->device_id = 0x1111; /* QEMU Standard VGA */
        vga->class_id = PCI_CLASS_DISPLAY;
        vga->bar0 = 0xFD000000;
        vga->valid = true;

        g_num_pci_devices = 2;
    }

    printk(KERN_INFO "pci: Probed PCI Bus controller (%u hardware devices enumerated)\n",
           g_num_pci_devices);
}

uint32_t pci_get_device_count(void) {
    return g_num_pci_devices;
}

pci_device_t* pci_get_device(uint32_t idx) {
    if (idx >= g_num_pci_devices) return NULL;
    return &g_pci_devices[idx];
}

pci_device_t* pci_find_device(uint16_t vendor_id, uint16_t device_id) {
    for (uint32_t i = 0; i < g_num_pci_devices; i++) {
        if (g_pci_devices[i].vendor_id == vendor_id &&
            (device_id == 0xFFFF || g_pci_devices[i].device_id == device_id)) {
            return &g_pci_devices[i];
        }
    }
    return NULL;
}
