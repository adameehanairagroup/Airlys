#ifndef AIRLYS_VBE_H
#define AIRLYS_VBE_H

/*
 * Airlys Operating System Kernel - VESA / VBE Graphics Engine (Phase 7 GUI)
 * Lead Architect: Er. Adam Eehan (Founder & CEO, Aira Group of Technology)
 * Co-Pilot: Sovereign AI Aira 3 Ultra (Engine v3.6)
 */

#include <airlys/types.h>

#define VBE_DEFAULT_WIDTH   1024
#define VBE_DEFAULT_HEIGHT  768
#define VBE_BPP             32

/* Sovereign Cyber-Fluent Color Palette (ARGB8888) */
#define COLOR_OBSIDIAN      0xFF060911
#define COLOR_DEEP_BG       0xFF0B101D
#define COLOR_CYAN_GLOW     0xFF00F2FE
#define COLOR_CYAN_DARK     0xFF006677
#define COLOR_FROSTED_GLASS 0xCC111827
#define COLOR_TITLEBAR_BG   0xEE0E1626
#define COLOR_TASKBAR_BG    0xDD070B14
#define COLOR_BORDER_CYAN   0xFF00A3C4
#define COLOR_WHITE         0xFFFFFFFF
#define COLOR_LIGHT_GREY    0xFFD1D5DB
#define COLOR_DARK_GREY     0xFF4B5563
#define COLOR_CRIMSON       0xFFFF3366
#define COLOR_GREEN_OK      0xFF10B981
#define COLOR_AMBER         0xFFF59E0B

typedef struct {
    uint32_t width;
    uint32_t height;
    uint32_t bpp;
    uint32_t pitch;
    uint32_t* front_buffer;
    uint32_t* back_buffer;
    bool     initialized;
} vbe_screen_t;

/* Graphics Engine API */
void vbe_init(uint32_t width, uint32_t height);
void vbe_set_physical_framebuffer(uint64_t addr, uint32_t width, uint32_t height, uint32_t pitch, uint32_t bpp);
bool vbe_is_hardware_framebuffer(void);
vbe_screen_t* vbe_get_screen(void);
void vbe_early_boot_splash(void);
void vbe_console_putchar(char c, uint32_t color);
void vbe_console_clear(void);
void vbe_allocate_backbuffer(void);

void vbe_putpixel(uint32_t x, uint32_t y, uint32_t color);
uint32_t vbe_getpixel(uint32_t x, uint32_t y);
void vbe_clear(uint32_t color);

void vbe_draw_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color);
void vbe_fill_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color);
void vbe_blend_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color, uint8_t alpha);
void vbe_draw_glow_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t border_color, uint32_t glow_color);

void vbe_draw_line(int x0, int y0, int x1, int y1, uint32_t color);
void vbe_draw_circle(int xc, int yc, int r, uint32_t color);
void vbe_fill_circle(int xc, int yc, int r, uint32_t color);

void vbe_swap_buffers(void);

#endif /* AIRLYS_VBE_H */
