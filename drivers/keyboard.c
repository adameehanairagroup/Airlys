#include <airlys/keyboard.h>
#include <airlys/idt.h>
#include <airlys/io.h>
#include <airlys/vga.h>
#include <airlys/pic.h>
#include <airlys/shell.h>
#include <airlys/mouse.h>
#include <airlys/compositor.h>

#define KBD_DATA_PORT 0x60

static bool shift_pressed = false;
static bool s_ext_scancode = false;
static bool ctrl_pressed = false;

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

    /* Check extended prefix */
    if (scancode == 0xE0) {
        s_ext_scancode = true;
        return;
    }

    if (s_ext_scancode) {
        s_ext_scancode = false;
        /* Extended scancodes for laptop keyboard */
        switch (scancode) {
            case 0x48: /* Up Arrow */
                mouse_move_relative(0, -20);
                return;
            case 0x50: /* Down Arrow */
                mouse_move_relative(0, 20);
                return;
            case 0x4B: /* Left Arrow */
                mouse_move_relative(-20, 0);
                return;
            case 0x4D: /* Right Arrow */
                mouse_move_relative(20, 0);
                return;
            case 0x5B: /* Super / Windows Key */
                compositor_toggle_start_menu();
                return;
            case 0x5D: /* Context Menu Key (Right Click) */
                mouse_set_buttons(false, true);
                mouse_set_buttons(false, false);
                return;
            default:
                break;
        }
        return;
    }

    /* Track Control key */
    if (scancode == 0x1D) {
        ctrl_pressed = true;
        return;
    }
    if (scancode == 0x9D) {
        ctrl_pressed = false;
        return;
    }

    /* Shift press / release */
    if (scancode == 0x2A || scancode == 0x36) {
        shift_pressed = true;
        return;
    }
    if (scancode == 0xAA || scancode == 0xB6) {
        shift_pressed = false;
        return;
    }

    /* Escape key (0x01): Dismiss menus */
    if (scancode == 0x01) {
        compositor_close_context_menu();
        return;
    }

    /* Enter key (0x1C) - Left Click trigger for cursor */
    if (scancode == 0x1C) {
        mouse_set_buttons(true, false);
    } else if (scancode == 0x9C) {
        mouse_set_buttons(false, false);
    }

    /* Ctrl+A: Toggle Action Center / Quick Settings */
    if (ctrl_pressed && scancode == 0x1E) {
        compositor_toggle_quick_settings();
        return;
    }

    /* Keypad navigation when NumLock is off */
    if (scancode == 0x48) { mouse_move_relative(0, -20); return; }
    if (scancode == 0x50) { mouse_move_relative(0, 20); return; }
    if (scancode == 0x4B) { mouse_move_relative(-20, 0); return; }
    if (scancode == 0x4D) { mouse_move_relative(20, 0); return; }

    /* Key release */
    if (scancode & 0x80) {
        return;
    }

    if (scancode < 128) {
        char c = shift_pressed ? kbd_scancode_shift[scancode] : kbd_scancode_normal[scancode];
        if (c != 0) {
            shell_handle_char(c);
        }
    }
}

void keyboard_init(void) {
    /* Register keyboard on IRQ1 (Vector 33) */
    register_interrupt_handler(33, keyboard_callback);

    /* Unmask IRQ1 on Master PIC */
    pic_unmask_irq(1);
}
