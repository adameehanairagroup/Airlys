#include <airlys/vbe.h>
#include <airlys/heap.h>
#include <airlys/string.h>
#include <airlys/printk.h>

static vbe_screen_t s_screen;
static bool s_is_hardware = false;

/* Static fallback buffer (1024 x 768 @ 32bpp) */
#define VBE_MAX_PIXELS (1024 * 768)
static uint32_t s_pixel_buffer[VBE_MAX_PIXELS];

bool vbe_is_hardware_framebuffer(void) {
    return s_is_hardware;
}

void vbe_set_physical_framebuffer(uint64_t addr, uint32_t width, uint32_t height, uint32_t pitch, uint32_t bpp) {
    if (width > 1920) width = 1920;
    if (height > 1080) height = 1080;
    if (width == 0) width = 1024;
    if (height == 0) height = 768;

    s_screen.width = width;
    s_screen.height = height;
    s_screen.pitch = (pitch > 0) ? pitch : width * 4;
    s_screen.bpp = (bpp > 0) ? bpp : 32;

    /* Point front_buffer directly to physical GPU MMIO memory */
    s_screen.front_buffer = (uint32_t*)addr;
    s_screen.back_buffer = (uint32_t*)kmalloc(width * height * sizeof(uint32_t));
    if (!s_screen.back_buffer) {
        s_screen.back_buffer = s_pixel_buffer;
    }

    s_screen.initialized = true;
    s_is_hardware = true;

    printk(KERN_INFO "vbe: Physical UEFI GOP Framebuffer mapped at 0x%lx (%ux%u, pitch: %u)\n",
           addr, width, height, s_screen.pitch);
}

void vbe_init(uint32_t width, uint32_t height) {
    if (s_is_hardware && s_screen.initialized) {
        return; /* Keep real hardware framebuffer intact */
    }

    if (width > 1024) width = 1024;
    if (height > 768) height = 768;

    s_screen.width = width;
    s_screen.height = height;
    s_screen.bpp = VBE_BPP;
    s_screen.pitch = width * 4;

    /* Primary display buffer and compositor backbuffer */
    s_screen.front_buffer = s_pixel_buffer;
    s_screen.back_buffer = (uint32_t*)kmalloc(width * height * sizeof(uint32_t));
    if (!s_screen.back_buffer) {
        s_screen.back_buffer = s_pixel_buffer;
    }

    s_screen.initialized = true;
    vbe_clear(COLOR_OBSIDIAN);
    vbe_swap_buffers();

    printk(KERN_INFO "vbe: VESA Framebuffer initialized (%ux%u @ 32bpp TrueColor)\n",
           width, height);
}

vbe_screen_t* vbe_get_screen(void) {
    return &s_screen;
}

void vbe_putpixel(uint32_t x, uint32_t y, uint32_t color) {
    if (!s_screen.initialized || x >= s_screen.width || y >= s_screen.height) {
        return;
    }
    s_screen.back_buffer[y * s_screen.width + x] = color;
}

uint32_t vbe_getpixel(uint32_t x, uint32_t y) {
    if (!s_screen.initialized || x >= s_screen.width || y >= s_screen.height) {
        return 0;
    }
    return s_screen.back_buffer[y * s_screen.width + x];
}

void vbe_clear(uint32_t color) {
    if (!s_screen.initialized) return;
    uint32_t total = s_screen.width * s_screen.height;
    for (uint32_t i = 0; i < total; i++) {
        s_screen.back_buffer[i] = color;
    }
}

void vbe_draw_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color) {
    for (uint32_t i = x; i < x + w; i++) {
        vbe_putpixel(i, y, color);
        vbe_putpixel(i, y + h - 1, color);
    }
    for (uint32_t j = y; j < y + h; j++) {
        vbe_putpixel(x, j, color);
        vbe_putpixel(x + w - 1, j, color);
    }
}

void vbe_fill_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color) {
    for (uint32_t j = y; j < y + h; j++) {
        for (uint32_t i = x; i < x + w; i++) {
            vbe_putpixel(i, j, color);
        }
    }
}

void vbe_blend_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color, uint8_t alpha) {
    if (!s_screen.initialized) return;
    uint32_t src_r = (color >> 16) & 0xFF;
    uint32_t src_g = (color >> 8) & 0xFF;
    uint32_t src_b = color & 0xFF;

    for (uint32_t j = y; j < y + h && j < s_screen.height; j++) {
        for (uint32_t i = x; i < x + w && i < s_screen.width; i++) {
            uint32_t dst = s_screen.back_buffer[j * s_screen.width + i];
            uint32_t dst_r = (dst >> 16) & 0xFF;
            uint32_t dst_g = (dst >> 8) & 0xFF;
            uint32_t dst_b = dst & 0xFF;

            uint32_t out_r = ((src_r * alpha) + (dst_r * (255 - alpha))) / 255;
            uint32_t out_g = ((src_g * alpha) + (dst_g * (255 - alpha))) / 255;
            uint32_t out_b = ((src_b * alpha) + (dst_b * (255 - alpha))) / 255;

            s_screen.back_buffer[j * s_screen.width + i] = 0xFF000000 | (out_r << 16) | (out_g << 8) | out_b;
        }
    }
}

void vbe_draw_glow_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t border_color, uint32_t glow_color) {
    /* Outer soft glow aura */
    if (x >= 2 && y >= 2 && x + w + 2 < s_screen.width && y + h + 2 < s_screen.height) {
        vbe_draw_rect(x - 2, y - 2, w + 4, h + 4, glow_color & 0x33FFFFFF);
        vbe_draw_rect(x - 1, y - 1, w + 2, h + 2, glow_color & 0x66FFFFFF);
    }
    /* Inner crisp 1px border */
    vbe_draw_rect(x, y, w, h, border_color);
}

void vbe_draw_line(int x0, int y0, int x1, int y1, uint32_t color) {
    int dx = x1 > x0 ? x1 - x0 : x0 - x1;
    int dy = y1 > y0 ? y1 - y0 : y0 - y1;
    int sx = x0 < x1 ? 1 : -1;
    int sy = y0 < y1 ? 1 : -1;
    int err = (dx > dy ? dx : -dy) / 2;

    for (;;) {
        vbe_putpixel((uint32_t)x0, (uint32_t)y0, color);
        if (x0 == x1 && y0 == y1) break;
        int e2 = err;
        if (e2 > -dx) { err -= dy; x0 += sx; }
        if (e2 < dy)  { err += dx; y0 += sy; }
    }
}

void vbe_draw_circle(int xc, int yc, int r, uint32_t color) {
    int x = 0;
    int y = r;
    int d = 3 - 2 * r;

    while (y >= x) {
        vbe_putpixel(xc + x, yc + y, color);
        vbe_putpixel(xc - x, yc + y, color);
        vbe_putpixel(xc + x, yc - y, color);
        vbe_putpixel(xc - x, yc - y, color);
        vbe_putpixel(xc + y, yc + x, color);
        vbe_putpixel(xc - y, yc + x, color);
        vbe_putpixel(xc + y, yc - x, color);
        vbe_putpixel(xc - y, yc - x, color);
        x++;
        if (d > 0) {
            y--;
            d = d + 4 * (x - y) + 10;
        } else {
            d = d + 4 * x + 6;
        }
    }
}

void vbe_fill_circle(int xc, int yc, int r, uint32_t color) {
    for (int y = -r; y <= r; y++) {
        for (int x = -r; x <= r; x++) {
            if (x * x + y * y <= r * r) {
                vbe_putpixel(xc + x, yc + y, color);
            }
        }
    }
}

void vbe_swap_buffers(void) {
    if (!s_screen.initialized || s_screen.front_buffer == s_screen.back_buffer) {
        return;
    }
    if (s_screen.pitch == s_screen.width * 4) {
        uint32_t total = s_screen.width * s_screen.height;
        kmemcpy(s_screen.front_buffer, s_screen.back_buffer, total * sizeof(uint32_t));
    } else {
        uint8_t* dst = (uint8_t*)s_screen.front_buffer;
        uint8_t* src = (uint8_t*)s_screen.back_buffer;
        uint32_t row_bytes = s_screen.width * 4;
        for (uint32_t y = 0; y < s_screen.height; y++) {
            kmemcpy(dst, src, row_bytes);
            dst += s_screen.pitch;
            src += row_bytes;
        }
    }
}
