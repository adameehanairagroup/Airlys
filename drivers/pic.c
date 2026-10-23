#include <airlys/pic.h>
#include <airlys/io.h>

#define ICW1_INIT    0x11
#define ICW4_8086    0x01

void pic_init(void) {
    uint8_t a1, a2;

    /* Save masks */
    a1 = inb(PIC1_DATA);
    a2 = inb(PIC2_DATA);

    /* Start initialization sequence in cascade mode (ICW1) */
    outb(PIC1_COMMAND, ICW1_INIT);
    io_wait();
    outb(PIC2_COMMAND, ICW1_INIT);
    io_wait();

    /* ICW2: Master PIC vector offset (32) and Slave PIC vector offset (40) */
    outb(PIC1_DATA, PIC1_OFFSET);
    io_wait();
    outb(PIC2_DATA, PIC2_OFFSET);
    io_wait();

    /* ICW3: Tell Master PIC there is a Slave PIC at IRQ2 (0000 0100) */
    outb(PIC1_DATA, 4);
    io_wait();
    /* Tell Slave PIC its cascade identity (0000 0010) */
    outb(PIC2_DATA, 2);
    io_wait();

    /* ICW4: Set 8086 mode */
    outb(PIC1_DATA, ICW4_8086);
    io_wait();
    outb(PIC2_DATA, ICW4_8086);
    io_wait();

    /* Restore masks (or mask all except cascade) */
    outb(PIC1_DATA, a1);
    outb(PIC2_DATA, a2);
}

void pic_send_eoi(uint8_t irq) {
    if (irq >= 8) {
        outb(PIC2_COMMAND, PIC_EOI);
    }
    outb(PIC1_COMMAND, PIC_EOI);
}

void pic_mask_irq(uint8_t irq) {
    uint16_t port;
    uint8_t value;

    if (irq < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq -= 8;
    }
    value = inb(port) | (uint8_t)(1 << irq);
    outb(port, value);
}

void pic_unmask_irq(uint8_t irq) {
    uint16_t port;
    uint8_t value;

    if (irq < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq -= 8;
    }
    value = inb(port) & (uint8_t)~(1 << irq);
    outb(port, value);
}
