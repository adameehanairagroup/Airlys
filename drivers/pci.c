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

/* Enable hardware 5V VBUS power to all USB Root Hub ports (xHCI / EHCI) */
void pci_enable_usb_port_power(void) {
    for (uint32_t i = 0; i < g_num_pci_devices; i++) {
        pci_device_t* dev = &g_pci_devices[i];
        if (dev->class_id != PCI_CLASS_SERIAL_BUS || dev->subclass_id != PCI_SUBCLASS_USB) {
            continue;
        }

        /* 1. Enable PCI Memory Space + Bus Master in Command Register (Offset 0x04) */
        uint16_t cmd = pci_read16(dev->bus, dev->slot, dev->func, 0x04);
        cmd |= (1 << 1) | (1 << 2); /* Memory Space Enable + Bus Master Enable */
        pci_write16(dev->bus, dev->slot, dev->func, 0x04, cmd);

        /* 2. Retrieve 64-bit MMIO Base Address */
        uint32_t bar0_low = pci_read32(dev->bus, dev->slot, dev->func, 0x10);
        uint64_t bar0_phys = bar0_low & ~0xFU;
        if ((bar0_low & 0x06) == 0x04) {
            /* 64-bit BAR: read upper 32 bits from BAR1 */
            uint32_t bar0_hi = pci_read32(dev->bus, dev->slot, dev->func, 0x14);
            bar0_phys |= ((uint64_t)bar0_hi << 32);
        }

        if (bar0_phys == 0 || bar0_phys == 0xFFFFFFF0) continue;

        /* 3. Handle xHCI (Extensible Host Controller Interface - USB 3.x) */
        if (dev->prog_if == PCI_PROGIF_XHCI) {
            volatile uint8_t* xhci_base = (volatile uint8_t*)bar0_phys;
            uint8_t caplength = xhci_base[0];
            uint32_t hcsparams1 = *(volatile uint32_t*)(xhci_base + 0x04);
            uint32_t max_ports = (hcsparams1 >> 24) & 0xFF;
            if (max_ports == 0 || max_ports > 32) max_ports = 16;

            volatile uint8_t* op_base = xhci_base + caplength;

            /* Turn on Run/Stop (R/S) bit in USBCMD (Offset 0x00) */
            volatile uint32_t* usbcmd = (volatile uint32_t*)(op_base + 0x00);
            *usbcmd |= (1U << 0);

            /* Turn on Port Power (PP = bit 9) on all root hub ports */
            volatile uint8_t* port_regs = op_base + 0x400;
            for (uint32_t p = 0; p < max_ports; p++) {
                volatile uint32_t* portsc = (volatile uint32_t*)(port_regs + (p * 0x10));
                uint32_t v = *portsc;
                /* Don't clear write-1-to-clear change bits */
                v &= ~((1U << 1) | (1U << 17) | (1U << 18) | (1U << 19) | (1U << 20) | (1U << 21) | (1U << 22));
                v |= (1U << 9); /* PP = 1: Turn ON 5V VBUS Power to USB port */
                *portsc = v;
            }

            /* Take ownership from BIOS/SMI if USBLEGSUP is present */
            uint32_t hccparams1 = *(volatile uint32_t*)(xhci_base + 0x10);
            uint32_t xecp = (hccparams1 >> 16) & 0xFFFF;
            if (xecp > 0) {
                volatile uint8_t* ext_base = xhci_base + (xecp << 2);
                uint32_t legsup = *(volatile uint32_t*)ext_base;
                if ((legsup & 0xFF) == 1) { /* USB Legacy Support ID = 1 */
                    *(volatile uint32_t*)ext_base |= (1U << 24); /* Claim OS Owned Semaphore */
                }
            }

            printk(KERN_INFO "pci: xHCI USB 3.x Host Controller initialized (Bus %u, %u Ports Powered)\n",
                   dev->bus, max_ports);
        }
        /* 4. Handle EHCI (Enhanced Host Controller Interface - USB 2.0) */
        else if (dev->prog_if == PCI_PROGIF_EHCI) {
            volatile uint8_t* ehci_base = (volatile uint8_t*)bar0_phys;
            uint8_t caplength = ehci_base[0];
            volatile uint8_t* op_base = ehci_base + caplength;

            /* Route ports to EHCI via CONFIGFLAG (Offset 0x40) */
            volatile uint32_t* configflag = (volatile uint32_t*)(op_base + 0x40);
            *configflag = 1;

            /* Turn on Port Power (PP = bit 12) */
            uint32_t hcsparams = *(volatile uint32_t*)(ehci_base + 0x04);
            uint32_t n_ports = hcsparams & 0x0F;
            if (n_ports == 0 || n_ports > 16) n_ports = 8;

            for (uint32_t p = 0; p < n_ports; p++) {
                volatile uint32_t* portsc = (volatile uint32_t*)(op_base + 0x44 + (p * 4));
                *portsc |= (1U << 12); /* PP = 1 */
            }

            printk(KERN_INFO "pci: EHCI USB 2.0 Host Controller initialized (Bus %u, %u Ports Powered)\n",
                   dev->bus, n_ports);
        }
    }
}

void pci_init(void) {
    g_num_pci_devices = 0;
    kmemset(g_pci_devices, 0, sizeof(g_pci_devices));

    /* Scan primary buses 0..31 (encompassing AMD Zen 3+ Root Complex & Hubs) */
    for (uint16_t bus = 0; bus < 32; bus++) {
        for (uint8_t slot = 0; slot < 32; slot++) {
            pci_check_device((uint8_t)bus, slot);
        }
    }

    /* Fallback default devices for emulation if none probed */
    if (g_num_pci_devices == 0) {
        pci_device_t* net = &g_pci_devices[0];
        net->bus = 0; net->slot = 3; net->func = 0;
        net->vendor_id = PCI_VENDOR_INTEL;
        net->device_id = 0x100E;
        net->class_id = PCI_CLASS_NETWORK;
        net->subclass_id = 0x00;
        net->bar0 = 0xF0000000;
        net->irq = 11;
        net->valid = true;

        pci_device_t* vga = &g_pci_devices[1];
        vga->bus = 0; vga->slot = 2; vga->func = 0;
        vga->vendor_id = PCI_VENDOR_QEMU;
        vga->device_id = 0x1111;
        vga->class_id = PCI_CLASS_DISPLAY;
        vga->bar0 = 0xFD000000;
        vga->valid = true;

        g_num_pci_devices = 2;
    }

    /* Turn on hardware power to all physical USB ports! */
    pci_enable_usb_port_power();

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
