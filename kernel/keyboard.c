#include <airlys/keyboard.h>
#include <airlys/idt.h>
#include <airlys/io.h>
#include <airlys/vga.h>
#include <airlys/pic.h>

#define KBD_DATA_PORT 0x60

static bool shift_pressed = false;

/* Scan Code Set 1 mapping (lower-case / normal) */
static const char kbd_scancode_normal[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, /* Control */
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, /* Left Shift */
    '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',
    0, /* Right Shift */
    '*',
    0,  /* Alt */
    ' ', /* Space */
    0,  /* Caps Lock */
};

/* Scan Code Set 1 mapping (Shifted / upper-case) */
static const char kbd_scancode_shift[128] = {
    0,  27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0, /* Control */
    'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '\"', '~',
    0, /* Left Shift */
    '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?',
    0, /* Right Shift */
    '*',
    0,  /* Alt */
    ' ', /* Space */
    0,  /* Caps Lock */
};

static void keyboard_callback(registers_t* regs) {
    (void)regs;
    uint8_t scancode = inb(KBD_DATA_PORT);

    /* Check for Shift key press (0x2A left, 0x36 right) */
    if (scancode == 0x2A || scancode == 0x36) {
        shift_pressed = true;
        return;
    }
    /* Check for Shift key release (0xAA left, 0xB6 right) */
    if (scancode == 0xAA || scancode == 0xB6) {
        shift_pressed = false;
        return;
    }

    /* Key release has bit 7 set (scancode >= 0x80) */
    if (scancode & 0x80) {
        return;
    }

    if (scancode < 128) {
        char c = shift_pressed ? kbd_scancode_shift[scancode] : kbd_scancode_normal[scancode];
        if (c != 0) {
            if (c == '\n') {
                vga_putchar('\n');
                vga_print_prompt();
            } else {
                vga_putchar(c);
            }
        }
    }
}

void keyboard_init(void) {
    /* Register keyboard on IRQ1 (Vector 33) */
    register_interrupt_handler(33, keyboard_callback);

    /* Unmask IRQ1 on Master PIC */
    pic_unmask_irq(1);
}
