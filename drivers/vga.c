#include <airlys/vga.h>
#include <airlys/vbe.h>
#include <airlys/serial.h>

static size_t vga_row;
static size_t vga_column;
static uint8_t vga_current_color;
static volatile uint16_t* const vga_buffer = (volatile uint16_t*)VGA_BUFFER_PHYSICAL;

static uint32_t vga_to_rgb(uint8_t vga_color) {
    uint8_t fg = vga_color & 0x0F;
    switch (fg) {
        case VGA_COLOR_BLACK:        return 0xFF000000;
        case VGA_COLOR_BLUE:         return 0xFF0055AA;
        case VGA_COLOR_GREEN:        return 0xFF00AA00;
        case VGA_COLOR_CYAN:         return 0xFF00AAAA;
        case VGA_COLOR_RED:          return 0xFFAA0000;
        case VGA_COLOR_MAGENTA:      return 0xFFAA00AA;
        case VGA_COLOR_BROWN:        return 0xFFAA5500;
        case VGA_COLOR_LIGHT_GREY:   return 0xFFAAAAAA;
        case VGA_COLOR_DARK_GREY:    return 0xFF555555;
        case VGA_COLOR_LIGHT_BLUE:   return 0xFF5555FF;
        case VGA_COLOR_LIGHT_GREEN:  return 0xFF55FF55;
        case VGA_COLOR_LIGHT_CYAN:   return 0xFF00E5FF;
        case VGA_COLOR_LIGHT_RED:    return 0xFFFF5555;
        case VGA_COLOR_LIGHT_MAGENTA:return 0xFFFF55FF;
        case VGA_COLOR_LIGHT_BROWN:  return 0xFFFFFF55;
        case VGA_COLOR_WHITE:        return 0xFFFFFFFF;
        default:                     return 0xFFFFFFFF;
    }
}

/* Outport byte to I/O port (used for hardware cursor updates) */
static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static void vga_update_cursor(void) {
    if (vbe_is_hardware_framebuffer()) return;
    uint16_t pos = (uint16_t)(vga_row * VGA_WIDTH + vga_column);
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

static void vga_scroll(void) {
    for (size_t y = 0; y < VGA_HEIGHT - 1; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            vga_buffer[y * VGA_WIDTH + x] = vga_buffer[(y + 1) * VGA_WIDTH + x];
        }
    }
    for (size_t x = 0; x < VGA_WIDTH; x++) {
        vga_buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = vga_entry(' ', vga_current_color);
    }
    vga_row = VGA_HEIGHT - 1;
}

void vga_init(void) {
    vga_row = 0;
    vga_column = 0;
    vga_current_color = vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_clear();
}

void vga_set_color(vga_color_t fg, vga_color_t bg) {
    vga_current_color = vga_entry_color(fg, bg);
}

void vga_clear(void) {
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            vga_buffer[y * VGA_WIDTH + x] = vga_entry(' ', vga_current_color);
        }
    }
    vga_row = 0;
    vga_column = 0;
    vga_update_cursor();
}

void vga_putchar(char c) {
    serial_putchar(c);

    if (vbe_is_hardware_framebuffer()) {
        vbe_console_putchar(c, vga_to_rgb(vga_current_color));
    }

    if (c == '\b') {
        if (vga_column > 12) {
            vga_column--;
            const size_t index = vga_row * VGA_WIDTH + vga_column;
            vga_buffer[index] = vga_entry(' ', vga_current_color);
            vga_update_cursor();
        }
        return;
    }

    if (c == '\n') {
        vga_column = 0;
        if (++vga_row == VGA_HEIGHT) {
            vga_scroll();
        }
        vga_update_cursor();
        return;
    }

    if (c == '\r') {
        vga_column = 0;
        vga_update_cursor();
        return;
    }

    if (c == '\t') {
        vga_column = (vga_column + 4) & ~3;
        if (vga_column >= VGA_WIDTH) {
            vga_column = 0;
            if (++vga_row == VGA_HEIGHT) {
                vga_scroll();
            }
        }
        vga_update_cursor();
        return;
    }

    const size_t index = vga_row * VGA_WIDTH + vga_column;
    vga_buffer[index] = vga_entry((unsigned char)c, vga_current_color);

    if (++vga_column == VGA_WIDTH) {
        vga_column = 0;
        if (++vga_row == VGA_HEIGHT) {
            vga_scroll();
        }
    }
    vga_update_cursor();
}

void vga_write(const char* data, size_t size) {
    for (size_t i = 0; i < size; i++) {
        vga_putchar(data[i]);
    }
}

void vga_puts(const char* str) {
    if (!str) return;
    while (*str) {
        vga_putchar(*str++);
    }
}

void vga_puthex(uint64_t val) {
    const char hex_chars[] = "0123456789ABCDEF";
    vga_puts("0x");
    for (int i = 60; i >= 0; i -= 4) {
        vga_putchar(hex_chars[(val >> i) & 0xF]);
    }
}

void vga_putdec(uint64_t val) {
    if (val == 0) {
        vga_putchar('0');
        return;
    }
    char buf[24];
    int i = 0;
    while (val > 0) {
        buf[i++] = (char)('0' + (val % 10));
        val /= 10;
    }
    while (--i >= 0) {
        vga_putchar(buf[i]);
    }
}

static char vga_prompt_path[64] = "/home/user";

void vga_set_prompt_path(const char* path) {
    if (!path) return;
    size_t i = 0;
    while (path[i] != '\0' && i < sizeof(vga_prompt_path) - 1) {
        vga_prompt_path[i] = path[i];
        i++;
    }
    vga_prompt_path[i] = '\0';
}

void vga_print_prompt(void) {
    /* Line 1: ┌──(root@airlys-os)-[current_path] */
    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_putchar((char)0xDA); /* CP437 box top-left: ┌ */
    vga_putchar((char)0xC4); /* CP437 horizontal: ─ */
    vga_putchar((char)0xC4); /* CP437 horizontal: ─ */
    vga_puts("(");

    vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
    vga_puts("root");

    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_puts("@airlys-os)-[");

    vga_set_color(VGA_COLOR_LIGHT_BLUE, VGA_COLOR_BLACK);
    vga_puts(vga_prompt_path);

    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_puts("]\n");

    /* Line 2: └─Airlys >> */
    vga_putchar((char)0xC0); /* CP437 box bottom-left: └ */
    vga_putchar((char)0xC4); /* CP437 horizontal: ─ */
    vga_puts("Airlys >> ");

    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
}
