#include <airlys/serial.h>
#include <airlys/io.h>

static bool s_com1_present = false;

void serial_init(void) {
    /* Test if COM1 actually exists using UART scratch register (offset +7) */
    outb(COM1_PORT + 7, 0x55);
    if (inb(COM1_PORT + 7) != 0x55) {
        s_com1_present = false;
        return;
    }
    outb(COM1_PORT + 7, 0xAA);
    if (inb(COM1_PORT + 7) != 0xAA) {
        s_com1_present = false;
        return;
    }

    s_com1_present = true;
    outb(COM1_PORT + 1, 0x00);    /* Disable all interrupts */
    outb(COM1_PORT + 3, 0x80);    /* Enable DLAB (set baud rate divisor) */
    outb(COM1_PORT + 0, 0x01);    /* Set divisor to 1 (115200 baud) */
    outb(COM1_PORT + 1, 0x00);
    outb(COM1_PORT + 3, 0x03);    /* 8 bits, no parity, one stop bit */
    outb(COM1_PORT + 2, 0xC7);    /* Enable FIFO, clear them, with 14-byte threshold */
    outb(COM1_PORT + 4, 0x0B);    /* IRQs enabled, RTS/DSR set */
}

static inline bool is_transmit_empty(void) {
    return (inb(COM1_PORT + 5) & 0x20) != 0;
}

void serial_putchar(char c) {
    if (!s_com1_present) return;

    uint32_t timeout = 50000;
    while (!is_transmit_empty() && --timeout > 0) {}
    if (timeout > 0) {
        outb(COM1_PORT, c);
    }
}

void serial_puts(const char* str) {
    if (!s_com1_present || !str) return;
    while (*str) {
        if (*str == '\n') {
            serial_putchar('\r');
        }
        serial_putchar(*str++);
    }
}
