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
bool compositor_is_start_menu_open(void);
bool compositor_is_lock_screen_open(void);

void compositor_render_frame(void);

#endif /* AIRLYS_COMPOSITOR_H */
