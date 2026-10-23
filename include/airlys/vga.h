#ifndef AIRLYS_VGA_H
#define AIRLYS_VGA_H

#include <airlys/types.h>

/* Hardware VGA text buffer at physical address 0xB8000 */
#define VGA_BUFFER_PHYSICAL 0xB8000
#define VGA_WIDTH  80
#define VGA_HEIGHT 25

typedef enum {
    VGA_COLOR_BLACK         = 0,
    VGA_COLOR_BLUE          = 1,
    VGA_COLOR_GREEN         = 2,
    VGA_COLOR_CYAN          = 3,
    VGA_COLOR_RED           = 4,
    VGA_COLOR_MAGENTA       = 5,
    VGA_COLOR_BROWN         = 6,
    VGA_COLOR_LIGHT_GREY    = 7,
    VGA_COLOR_DARK_GREY     = 8,
    VGA_COLOR_LIGHT_BLUE    = 9,
    VGA_COLOR_LIGHT_GREEN   = 10,
    VGA_COLOR_LIGHT_CYAN    = 11,
    VGA_COLOR_LIGHT_RED     = 12,
    VGA_COLOR_LIGHT_MAGENTA = 13,
    VGA_COLOR_LIGHT_BROWN   = 14,
    VGA_COLOR_WHITE         = 15,
} vga_color_t;

static inline uint8_t vga_entry_color(vga_color_t fg, vga_color_t bg) {
    return (uint8_t)(fg | (bg << 4));
}

static inline uint16_t vga_entry(unsigned char uc, uint8_t color) {
    return (uint16_t)uc | ((uint16_t)color << 8);
}

void vga_init(void);
void vga_set_color(vga_color_t fg, vga_color_t bg);
void vga_clear(void);
void vga_putchar(char c);
void vga_write(const char* data, size_t size);
void vga_puts(const char* str);
void vga_puthex(uint64_t val);
void vga_putdec(uint64_t val);

#endif /* AIRLYS_VGA_H */
