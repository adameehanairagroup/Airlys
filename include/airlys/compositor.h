#ifndef AIRLYS_COMPOSITOR_H
#define AIRLYS_COMPOSITOR_H

/*
 * Airlys Operating System Kernel - Desktop Compositor & Window Manager (Phase 7 GUI)
 * Lead Architect: Er. Adam Eehan (Founder & CEO, Aira Group of Technology)
 * Co-Pilot: Sovereign AI Aira 3 Ultra (Engine v3.6)
 */

#include <airlys/types.h>
#include <airlys/vbe.h>

#define MAX_WINDOWS 8

#define WALLPAPER_DEFAULT_LOGO   0  /* Authentic Sovereign Logo on Obsidian */
#define WALLPAPER_CYBER_AURORA   1  /* Vibrant Cyan-Purple Aurora Vector */
#define WALLPAPER_MINIMAL_DARK   2  /* Pure Matte Stealth Dark */
#define WALLPAPER_EMERALD_MATRIX 3  /* Cyber Emerald Matrix */

struct window;
typedef void (*window_draw_fn)(struct window* win);

typedef struct window {
    uint32_t       id;
    char           title[64];
    uint32_t       x;
    uint32_t       y;
    uint32_t       w;
    uint32_t       h;
    bool           active;
    bool           minimized;
    bool           maximized;
    window_draw_fn draw_content;
    void*          user_data;
} window_t;

/* Compositor API */
void compositor_init(void);
window_t* compositor_create_window(const char* title, uint32_t x, uint32_t y, uint32_t w, uint32_t h, window_draw_fn draw);
void compositor_close_window(uint32_t id);
void compositor_set_active_window(uint32_t id);

void compositor_toggle_start_menu(void);
void compositor_toggle_lock_screen(void);
void compositor_toggle_quick_settings(void);
void compositor_close_context_menu(void);

bool compositor_is_start_menu_open(void);
bool compositor_is_lock_screen_open(void);
bool compositor_is_quick_settings_open(void);

void compositor_set_wallpaper(uint32_t wallpaper_id);
uint32_t compositor_get_wallpaper(void);

void compositor_handle_mouse(int32_t x, int32_t y, bool left, bool right);
void compositor_render_frame(void);

#endif /* AIRLYS_COMPOSITOR_H */
