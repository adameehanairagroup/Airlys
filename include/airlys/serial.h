#ifndef AIRLYS_SERIAL_H
#define AIRLYS_SERIAL_H

#include <airlys/types.h>

#define COM1_PORT 0x3F8

/* Initialize 16550A UART Serial Port (COM1) */
void serial_init(void);

/* Write a single character to serial port */
void serial_putchar(char c);

/* Write a string to serial port */
void serial_puts(const char* str);

#endif /* AIRLYS_SERIAL_H */
