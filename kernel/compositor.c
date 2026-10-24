#include <airlys/compositor.h>
#include <airlys/apps.h>
#include <airlys/mouse.h>
#include <airlys/font.h>
#include <airlys/rtc.h>
#include <airlys/timer.h>
#include <airlys/string.h>
#include <airlys/acpi.h>
#include <airlys/wallpaper_logo.h>

#define COLOR_AMBER_GLOW     0xFFFFB800

static window_t g_windows[MAX_WINDOWS];
static uint32_t g_num_windows = 0;
static uint32_t g_active_window_id = 0;
static bool     g_start_menu_open = false;
static bool     g_lock_screen_open = false;

/* Wallpaper state */
static uint32_t g_active_wallpaper = WALLPAPER_DEFAULT_LOGO;

/* Context Menu state */
static bool     g_context_menu_open = false;
static int32_t  g_context_x = 0;
static int32_t  g_context_y = 0;

/* Quick Settings (Action Center) state */
static bool     g_quick_settings_open = false;
static bool     g_qs_wifi = true;
static bool     g_qs_bt = true;
static bool     g_qs_airplane = false;
static bool     g_qs_night_light = false;
static bool     g_qs_battery_saver = false;
static uint32_t g_qs_volume = 80;
static uint32_t g_qs_brightness = 100;

/* Mouse interaction state */
static bool     s_prev_left = false;
static bool     s_prev_right = false;
static uint32_t s_drag_win_id = 0;
static int32_t  s_drag_ox = 0;
static int32_t  s_drag_oy = 0;

/* ---------------- Forward Declarations ---------------- */
static void draw_window(window_t* win);
static void draw_desktop_icons(vbe_screen_t* scr);
static void draw_taskbar(vbe_screen_t* scr);
static void draw_start_menu(vbe_screen_t* scr);
static void draw_context_menu(vbe_screen_t* scr);
static void draw_quick_settings(vbe_screen_t* scr);
static void draw_mouse_cursor(vbe_screen_t* scr, int32_t mx, int32_t my);

/* ---------------- Wallpaper API ---------------- */

void compositor_set_wallpaper(uint32_t wallpaper_id) {
    g_active_wallpaper = wallpaper_id % 4;
}

uint32_t compositor_get_wallpaper(void) {
    return g_active_wallpaper;
}

void compositor_toggle_quick_settings(void) {
    g_quick_settings_open = !g_quick_settings_open;
    if (g_quick_settings_open) {
        g_start_menu_open = false;
        g_context_menu_open = false;
    }
}

bool compositor_is_quick_settings_open(void) {
    return g_quick_settings_open;
}

void compositor_close_context_menu(void) {
    g_context_menu_open = false;
    g_quick_settings_open = false;
    g_start_menu_open = false;
}

/* ---------------- Compositor Lifecycle ---------------- */

void compositor_init(void) {
    g_num_windows = 0;
    g_active_window_id = 0;
    g_start_menu_open = false;
    g_lock_screen_open = false;
    g_context_menu_open = false;
    g_quick_settings_open = false;
    g_active_wallpaper = WALLPAPER_DEFAULT_LOGO;
    kmemset(g_windows, 0, sizeof(g_windows));

    /* Initialize VBE backbuffer */
    vbe_init(1024, 768);
    vbe_allocate_backbuffer();
    apps_init();

    /* Automatically launch Sovereign Hub on boot so desktop is rich and welcoming! */
    app_launch_hub();
}

window_t* compositor_create_window(const char* title, uint32_t x, uint32_t y, uint32_t w, uint32_t h, window_draw_fn draw) {
    if (g_num_windows >= MAX_WINDOWS) return NULL;

    for (int i = 0; i < MAX_WINDOWS; i++) {
        if (!g_windows[i].active) {
            window_t* win = &g_windows[i];
            win->id = (uint32_t)i + 1;
            kstrncpy(win->title, title, sizeof(win->title));
            win->x = x;
            win->y = y;
            win->w = w;
            win->h = h;
            win->active = true;
            win->minimized = false;
            win->maximized = false;
            win->draw_content = draw;
            win->user_data = NULL;

            g_active_window_id = win->id;
            g_num_windows++;
            return win;
        }
    }
    return NULL;
}

void compositor_close_window(uint32_t id) {
    for (int i = 0; i < MAX_WINDOWS; i++) {
        if (g_windows[i].active && g_windows[i].id == id) {
            g_windows[i].active = false;
            if (g_num_windows > 0) g_num_windows--;
            if (g_active_window_id == id) {
                g_active_window_id = 0;
                for (int j = MAX_WINDOWS - 1; j >= 0; j--) {
                    if (g_windows[j].active && !g_windows[j].minimized) {
                        g_active_window_id = g_windows[j].id;
                        break;
                    }
                }
            }
            break;
        }
    }
}

void compositor_set_active_window(uint32_t id) {
    g_active_window_id = id;
}

void compositor_toggle_start_menu(void) {
    g_start_menu_open = !g_start_menu_open;
    if (g_start_menu_open) {
        g_quick_settings_open = false;
        g_context_menu_open = false;
    }
}

void compositor_toggle_lock_screen(void) {
    g_lock_screen_open = !g_lock_screen_open;
}

bool compositor_is_start_menu_open(void) {
    return g_start_menu_open;
}

bool compositor_is_lock_screen_open(void) {
    return g_lock_screen_open;
}

/* ---------------- Vector Graphics & Icons ---------------- */

static void draw_icon_start(uint32_t x, uint32_t y) {
    vbe_fill_rect(x, y, 32, 32, 0xFF0B1424);
    vbe_draw_glow_rect(x, y, 32, 32, COLOR_CYAN_GLOW, COLOR_CYAN_DARK);

    /* Sovereign Eagle Crest */
    vbe_fill_circle(x + 16, y + 16, 9, 0xFF003344);
    vbe_draw_line(x + 7,  y + 13, x + 16, y + 7,  COLOR_CYAN_GLOW);
    vbe_draw_line(x + 16, y + 7,  x + 25, y + 13, COLOR_CYAN_GLOW);
    vbe_draw_line(x + 7,  y + 13, x + 12, y + 21, COLOR_CYAN_GLOW);
    vbe_draw_line(x + 25, y + 13, x + 20, y + 21, COLOR_CYAN_GLOW);
    vbe_draw_line(x + 12, y + 21, x + 16, y + 26, COLOR_CYAN_GLOW);
    vbe_draw_line(x + 20, y + 21, x + 16, y + 26, COLOR_CYAN_GLOW);
}

static void draw_icon_terminal(uint32_t x, uint32_t y) {
    vbe_fill_rect(x + 3, y + 3, 26, 22, 0xFF0A0F1D);
    vbe_draw_rect(x + 3, y + 3, 26, 22, COLOR_BORDER_CYAN);
    vbe_draw_line(x + 7, y + 9, x + 12, y + 14, COLOR_CYAN_GLOW);
    vbe_draw_line(x + 12, y + 14, x + 7, y + 19, COLOR_CYAN_GLOW);
    vbe_draw_line(x + 15, y + 19, x + 23, y + 19, COLOR_GREEN_OK);
}

static void draw_icon_files(uint32_t x, uint32_t y) {
    vbe_fill_rect(x + 4, y + 8, 24, 18, 0xFF0D2137);
    vbe_draw_rect(x + 4, y + 8, 24, 18, COLOR_BORDER_CYAN);
    vbe_fill_rect(x + 4, y + 4, 12, 6, 0xFF005577);
    vbe_draw_rect(x + 4, y + 4, 12, 6, COLOR_CYAN_GLOW);
    vbe_draw_line(x + 8, y + 13, x + 24, y + 13, COLOR_CYAN_GLOW);
    vbe_draw_line(x + 8, y + 17, x + 20, y + 17, COLOR_LIGHT_GREY);
}

static void draw_icon_code(uint32_t x, uint32_t y) {
    vbe_fill_rect(x + 3, y + 3, 26, 22, 0xFF140D26);
    vbe_draw_rect(x + 3, y + 3, 26, 22, 0xFFA855F7);
    vbe_draw_line(x + 9, y + 14, x + 6, y + 14, 0xFFA855F7);
    vbe_draw_line(x + 9, y + 9, x + 6, y + 14, 0xFFA855F7);
    vbe_draw_line(x + 6, y + 14, x + 9, y + 19, 0xFFA855F7);
    vbe_draw_line(x + 17, y + 8, x + 13, y + 20, COLOR_WHITE);
    vbe_draw_line(x + 21, y + 9, x + 24, y + 14, 0xFFA855F7);
    vbe_draw_line(x + 24, y + 14, x + 21, y + 19, 0xFFA855F7);
}

static void draw_icon_store(uint32_t x, uint32_t y) {
    vbe_fill_rect(x + 3, y + 3, 26, 22, 0xFF0C2226);
    vbe_draw_rect(x + 3, y + 3, 26, 22, COLOR_BORDER_CYAN);
    vbe_fill_rect(x + 7, y + 11, 18, 12, 0xFF0D9488);
    vbe_draw_line(x + 12, y + 8, x + 14, y + 6, COLOR_CYAN_GLOW);
    vbe_draw_line(x + 14, y + 6, x + 18, y + 6, COLOR_CYAN_GLOW);
    vbe_draw_line(x + 18, y + 6, x + 20, y + 8, COLOR_CYAN_GLOW);
    vbe_fill_circle(x + 16, y + 16, 2, 0xFFFF007F);
}

static void draw_icon_taskmgr(uint32_t x, uint32_t y) {
    vbe_fill_rect(x + 3, y + 3, 26, 22, 0xFF071924);
    vbe_draw_rect(x + 3, y + 3, 26, 22, COLOR_BORDER_CYAN);
    vbe_draw_line(x + 5, y + 14, x + 9, y + 14, 0xFF00FF88);
    vbe_draw_line(x + 9, y + 14, x + 12, y + 19, 0xFF00FF88);
    vbe_draw_line(x + 12, y + 19, x + 15, y + 8, 0xFF00FF88);
    vbe_draw_line(x + 15, y + 8, x + 18, y + 21, 0xFF00FF88);
    vbe_draw_line(x + 18, y + 21, x + 21, y + 14, 0xFF00FF88);
    vbe_draw_line(x + 21, y + 14, x + 27, y + 14, 0xFF00FF88);
}

static void draw_icon_search(uint32_t x, uint32_t y) {
    vbe_fill_rect(x + 3, y + 3, 26, 22, 0xFF10192A);
    vbe_draw_rect(x + 3, y + 3, 26, 22, COLOR_BORDER_CYAN);
    vbe_draw_circle(x + 13, y + 12, 5, COLOR_CYAN_GLOW);
    vbe_draw_line(x + 17, y + 16, x + 23, y + 21, COLOR_CYAN_GLOW);
}

static void draw_icon_settings(uint32_t x, uint32_t y) {
    vbe_fill_rect(x + 3, y + 3, 26, 22, 0xFF161F2E);
    vbe_draw_rect(x + 3, y + 3, 26, 22, COLOR_BORDER_CYAN);
    vbe_draw_circle(x + 16, y + 14, 4, COLOR_CYAN_GLOW);
    vbe_draw_line(x + 16, y + 7, x + 16, y + 10, COLOR_CYAN_GLOW);
    vbe_draw_line(x + 16, y + 18, x + 16, y + 21, COLOR_CYAN_GLOW);
    vbe_draw_line(x + 9, y + 14, x + 12, y + 14, COLOR_CYAN_GLOW);
    vbe_draw_line(x + 20, y + 14, x + 23, y + 14, COLOR_CYAN_GLOW);
}

/* ---------------- Full-Frame Edge-to-Edge Desktop Wallpaper ---------------- */

static void draw_desktop_wallpaper(vbe_screen_t* scr) {
    uint32_t w = scr->width;
    uint32_t h = scr->height;

    if (g_active_wallpaper == WALLPAPER_DEFAULT_LOGO) {
        /*
         * Theme 0: Authentic Sovereign Cyber Nebula (Edge-to-Edge Full-Frame)
         * Multi-stage gradient from deep space down to royal sapphire nebula & electric horizon
         */
        for (uint32_t y = 0; y < h; y += 4) {
            uint32_t color;
            if (y < h / 3) {
                color = 0xFF060B16; /* Deep obsidian space */
            } else if (y < (2 * h) / 3) {
                color = 0xFF0A182E; /* Royal sapphire nebula */
            } else {
                color = 0xFF050E1A; /* Ground horizon */
            }
            vbe_fill_rect(0, y, w, 4, color);
        }

        /* Subtle radiant aurora rays */
        for (uint32_t x = 0; x < w; x += 32) {
            vbe_draw_line(x, h / 2 + 100, x + 120, h / 2 - 120, 0x1200F2FE);
        }

        /* Center the exact Airlys Logo created by Er. Adam Eehan */
        int32_t lx = ((int32_t)w - WALLPAPER_LOGO_W) / 2;
        int32_t ly = ((int32_t)h - WALLPAPER_LOGO_H) / 2 - 20;

        for (int y = 0; y < WALLPAPER_LOGO_H; y++) {
            int py = ly + y;
            if (py < 0 || py >= (int)h) continue;
            uint32_t row_offset = y * WALLPAPER_LOGO_W;
            for (int x = 0; x < WALLPAPER_LOGO_W; x++) {
                int px = lx + x;
                if (px < 0 || px >= (int)w) continue;
                uint32_t color = g_wallpaper_logo_pixels[row_offset + x];
                vbe_putpixel(px, py, color);
            }
        }

        /* Sovereign Brand Title below logo */
        uint32_t cx = w / 2;
        uint32_t cy = (h / 2) - 20;
        const char* brand = "AIRLYS SOVEREIGN OPERATING SYSTEM";
        const char* sub   = "Engineered by Er. Adam Eehan & Sovereign AI Aira";
        font_draw_string(cx - (kstrlen(brand) * 4), cy + 130, brand, COLOR_CYAN_GLOW);
        font_draw_string(cx - (kstrlen(sub) * 4), cy + 150, sub, COLOR_LIGHT_GREY);

    } else if (g_active_wallpaper == WALLPAPER_CYBER_AURORA) {
        /* Theme 1: Electric Cyan Aurora */
        for (uint32_t y = 0; y < h; y += 4) {
            uint32_t col = (y < h / 2) ? 0xFF0B1426 : 0xFF050810;
            vbe_fill_rect(0, y, w, 4, col);
        }
        for (uint32_t x = 0; x < w; x += 16) {
            vbe_draw_line(x, h / 2, x + 80, (h / 2) - 100, 0x1A00F2FE);
            vbe_draw_line(x, (h / 2) + 60, x + 60, (h / 2) - 40, 0x12A855F7);
        }
        const char* title = "AIRLYS CYBER AURORA";
        font_draw_string((w / 2) - (kstrlen(title) * 4), (h / 2) - 10, title, COLOR_CYAN_GLOW);

    } else if (g_active_wallpaper == WALLPAPER_MINIMAL_DARK) {
        /* Theme 2: Pure Matte Stealth Dark */
        vbe_fill_rect(0, 0, w, h, 0xFF08090C);
        vbe_draw_line((w / 2) - 180, h / 2, (w / 2) + 180, h / 2, COLOR_BORDER_CYAN);
        const char* title = "A I R L Y S   S O V E R E I G N";
        font_draw_string((w / 2) - (kstrlen(title) * 4), (h / 2) - 20, title, COLOR_WHITE);

    } else if (g_active_wallpaper == WALLPAPER_EMERALD_MATRIX) {
        /* Theme 3: Cyber Emerald Matrix */
        vbe_fill_rect(0, 0, w, h, 0xFF030D08);
        for (uint32_t y = 0; y < h; y += 40) {
            vbe_draw_line(0, y, w, y, 0xFF082215);
        }
        for (uint32_t x = 0; x < w; x += 60) {
            vbe_draw_line(x, 0, x, h, 0xFF082215);
        }
        const char* title = "AIRLYS EMERALD CORE";
        font_draw_string((w / 2) - (kstrlen(title) * 4), (h / 2) - 10, title, COLOR_GREEN_OK);
    }
}

/* ---------------- Desktop Quick Icons (LVGL Grid) ---------------- */

static void draw_desktop_icons(vbe_screen_t* scr) {
    (void)scr;
    const char* icon_titles[] = { "This PC", "Files", "Task Manager", "Aira Search", "Settings" };
    uint32_t iy = 30;

    for (int i = 0; i < 5; i++) {
        uint32_t ix = 25;

        /* Frosted Tile */
        vbe_fill_rect(ix, iy, 56, 56, 0xBB0D1B2A);
        vbe_draw_rect(ix, iy, 56, 56, COLOR_BORDER_CYAN);

        /* Icon Graphic */
        if (i == 0) {
            /* This PC (Computer Monitor) */
            vbe_draw_rect(ix + 12, iy + 10, 32, 22, COLOR_CYAN_GLOW);
            vbe_fill_rect(ix + 14, iy + 12, 28, 18, 0xFF0A1828);
            vbe_draw_line(ix + 28, iy + 32, ix + 28, iy + 38, COLOR_CYAN_GLOW);
            vbe_draw_line(ix + 20, iy + 38, ix + 36, iy + 38, COLOR_CYAN_GLOW);
        } else if (i == 1) {
            draw_icon_files(ix + 12, iy + 14);
        } else if (i == 2) {
            draw_icon_taskmgr(ix + 12, iy + 14);
        } else if (i == 3) {
            draw_icon_search(ix + 12, iy + 14);
        } else if (i == 4) {
            draw_icon_settings(ix + 12, iy + 14);
        }

        /* Label Underneath */
        font_draw_string(ix - 8, iy + 60, icon_titles[i], COLOR_WHITE);

        iy += 84;
    }
}

/* ---------------- Window Drawing ---------------- */

static void draw_window(window_t* win) {
    if (!win->active || win->minimized) return;

    /* Frosted Acrylic Window Body */
    vbe_fill_rect(win->x, win->y, win->w, win->h, COLOR_FROSTED_GLASS);

    /* Window Titlebar */
    uint32_t tb_color = (win->id == g_active_window_id) ? COLOR_TITLEBAR_BG : 0xEE090E18;
    vbe_fill_rect(win->x, win->y, win->w, 28, tb_color);
    vbe_draw_line(win->x, win->y + 28, win->x + win->w, win->y + 28, COLOR_BORDER_CYAN);

    /* Title string */
    font_draw_string(win->x + 12, win->y + 6, win->title,
                     (win->id == g_active_window_id) ? COLOR_CYAN_GLOW : COLOR_LIGHT_GREY);

    /* Control Buttons (— [ ] ✕) */
    uint32_t btn_x = win->x + win->w - 65;
    font_draw_string(btn_x, win->y + 6, "-", COLOR_LIGHT_GREY);
    font_draw_string(btn_x + 20, win->y + 6, "[ ]", COLOR_LIGHT_GREY);

    /* Close button */
    vbe_fill_rect(btn_x + 42, win->y + 4, 18, 20, 0x33FF3366);
    font_draw_string(btn_x + 47, win->y + 6, "x", COLOR_CRIMSON);

    /* Window Content Area */
    if (win->draw_content) {
        win->draw_content(win);
    }

    /* Glow Border */
    uint32_t border_color = (win->id == g_active_window_id) ? COLOR_CYAN_GLOW : COLOR_BORDER_CYAN;
    vbe_draw_glow_rect(win->x, win->y, win->w, win->h, border_color, COLOR_CYAN_DARK);
}

/* ---------------- Full-Width Windows 11 Taskbar Dock ---------------- */

static void draw_taskbar(vbe_screen_t* scr) {
    uint32_t bar_h = 48;
    uint32_t bar_y = scr->height - bar_h;
    uint32_t bar_w = scr->width;

    /* Full-Width Translucent Acrylic Taskbar */
    vbe_fill_rect(0, bar_y, bar_w, bar_h, COLOR_TASKBAR_BG);
    vbe_draw_line(0, bar_y, bar_w, bar_y, COLOR_BORDER_CYAN); /* Top 1px highlight */

    /* Left: Start Button */
    uint32_t sx = 14;
    uint32_t sy = bar_y + 8;
    draw_icon_start(sx, sy);

    /* Left: Taskbar Search Box */
    uint32_t sb_x = sx + 42;
    uint32_t sb_w = 210;
    vbe_fill_rect(sb_x, bar_y + 8, sb_w, 32, 0xFF080D18);
    vbe_draw_rect(sb_x, bar_y + 8, sb_w, 32, COLOR_BORDER_CYAN);
    font_draw_string(sb_x + 12, bar_y + 17, "Type to search...", COLOR_LIGHT_GREY);

    /* Center: Pinned & Running App Icons */
    uint32_t app_center_w = 7 * 54;
    uint32_t app_x = (bar_w / 2) - (app_center_w / 2);

    for (int i = 0; i < 7; i++) {
        uint32_t tx = app_x + (i * 54);
        uint32_t ty = bar_y + 10;

        if (i == 0) draw_icon_terminal(tx, ty);
        else if (i == 1) draw_icon_files(tx, ty);
        else if (i == 2) draw_icon_code(tx, ty);
        else if (i == 3) draw_icon_store(tx, ty);
        else if (i == 4) draw_icon_taskmgr(tx, ty);
        else if (i == 5) draw_icon_search(tx, ty);
        else if (i == 6) draw_icon_settings(tx, ty);

        /* Running Indicator Bar */
        bool is_open = false;
        bool is_active = false;
        for (int w = 0; w < MAX_WINDOWS; w++) {
            if (!g_windows[w].active) continue;
            if ((i == 0 && kstrcmp(g_windows[w].title, "airlys-term") == 0) ||
                (i == 1 && kstrcmp(g_windows[w].title, "airlys-files") == 0) ||
                (i == 2 && kstrcmp(g_windows[w].title, "aira-code") == 0) ||
                (i == 4 && kstrcmp(g_windows[w].title, "Aira Task Manager") == 0) ||
                (i == 5 && kstrncmp(g_windows[w].title, "Airlys", 6) == 0) ||
                (i == 6 && kstrncmp(g_windows[w].title, "Airlys Sovereign Hub", 20) == 0)) {
                is_open = true;
                if (g_windows[w].id == g_active_window_id && !g_windows[w].minimized) {
                    is_active = true;
                }
                break;
            }
        }

        if (is_active) {
            vbe_fill_rect(tx + 8, ty + 28, 16, 2, COLOR_CYAN_GLOW);
        } else if (is_open) {
            vbe_fill_rect(tx + 12, ty + 28, 8, 2, COLOR_LIGHT_GREY);
        }
    }

    /* Right: System Tray */
    uint32_t tray_w = 260;
    uint32_t tray_x = bar_w - tray_w - 10;
    vbe_fill_rect(tray_x, bar_y + 6, tray_w, 36, 0xEE0B1424);
    vbe_draw_rect(tray_x, bar_y + 6, tray_w, 36, COLOR_BORDER_CYAN);

    rtc_time_t t = rtc_get_time();
    char clk_buf[16];
    clk_buf[0] = '0' + (t.hour / 10);
    clk_buf[1] = '0' + (t.hour % 10);
    clk_buf[2] = ':';
    clk_buf[3] = '0' + (t.minute / 10);
    clk_buf[4] = '0' + (t.minute % 10);
    clk_buf[5] = '\0';

    font_draw_string(tray_x + 12, bar_y + 17, clk_buf, COLOR_CYAN_GLOW);
    font_draw_string(tray_x + 65, bar_y + 17, "[WiFi 5G] [98% ⚡] [Vol]", COLOR_WHITE);
}

/* ---------------- Start Menu ---------------- */

static void draw_start_menu(vbe_screen_t* scr) {
    if (!g_start_menu_open) return;

    uint32_t sm_w = 460;
    uint32_t sm_h = 440;
    uint32_t sm_x = 20;
    uint32_t sm_y = scr->height - 58 - sm_h;

    vbe_fill_rect(sm_x, sm_y, sm_w, sm_h, 0xFA0B111E);
    vbe_draw_glow_rect(sm_x, sm_y, sm_w, sm_h, COLOR_CYAN_GLOW, COLOR_CYAN_DARK);

    /* Search Bar */
    uint32_t sb_x = sm_x + 20;
    uint32_t sb_y = sm_y + 18;
    vbe_fill_rect(sb_x, sb_y, sm_w - 40, 32, 0xFF070B14);
    vbe_draw_rect(sb_x, sb_y, sm_w - 40, 32, COLOR_BORDER_CYAN);
    font_draw_string(sb_x + 12, sb_y + 8, "Search apps, files, or ask Aira AI...", COLOR_LIGHT_GREY);

    /* Pinned Apps */
    uint32_t py = sm_y + 65;
    font_draw_string(sm_x + 20, py, "Pinned Applications", COLOR_CYAN_GLOW);
    py += 24;

    const char* pinned[] = {
        "Sovereign Hub", "File Explorer", "Aira Code", "AiraStore",
        "Task Manager", "Terminal", "Aira Search", "Settings"
    };

    for (int r = 0; r < 2; r++) {
        for (int c = 0; c < 4; c++) {
            uint32_t ax = sm_x + 20 + (c * 105);
            uint32_t ay = py + (r * 72);
            vbe_fill_rect(ax, ay, 96, 60, 0xFF0B1424);
            vbe_draw_rect(ax, ay, 96, 60, COLOR_BORDER_CYAN);

            int idx = r * 4 + c;
            font_draw_string(ax + 12, ay + 14, "[App]", COLOR_CYAN_GLOW);
            font_draw_string(ax + 8, ay + 36, pinned[idx], COLOR_WHITE);
        }
    }

    /* Bottom User Profile */
    uint32_t uy = sm_y + sm_h - 44;
    vbe_fill_rect(sm_x, uy, sm_w, 44, 0xFF070B14);
    vbe_draw_line(sm_x, uy, sm_x + sm_w, uy, COLOR_BORDER_CYAN);

    vbe_draw_circle(sm_x + 25, uy + 22, 12, COLOR_CYAN_GLOW);
    font_draw_char(sm_x + 22, uy + 14, 'A', COLOR_CYAN_GLOW);
    font_draw_string(sm_x + 45, uy + 8, "Er. Adam Eehan", COLOR_WHITE);
    font_draw_string(sm_x + 45, uy + 24, "Founder & CEO, Aira Group", COLOR_GREEN_OK);

    font_draw_string(sm_x + sm_w - 75, uy + 15, "[Power]", COLOR_CRIMSON);
}

/* ---------------- Right-Click Context Menu ---------------- */

static void draw_context_menu(vbe_screen_t* scr) {
    if (!g_context_menu_open) return;

    uint32_t cm_w = 195;
    uint32_t cm_h = 145;
    int32_t  cm_x = g_context_x;
    int32_t  cm_y = g_context_y;

    if (cm_x + (int32_t)cm_w > (int32_t)scr->width) cm_x = scr->width - cm_w - 10;
    if (cm_y + (int32_t)cm_h > (int32_t)scr->height - 50) cm_y = scr->height - 50 - cm_h;

    vbe_fill_rect(cm_x, cm_y, cm_w, cm_h, 0xFA0B111E);
    vbe_draw_glow_rect(cm_x, cm_y, cm_w, cm_h, COLOR_CYAN_GLOW, COLOR_CYAN_DARK);

    const char* items[] = {
        "[W] Change Wallpaper",
        "[*] Sovereign Hub",
        "[>] Open Terminal",
        "[#] Aira Task Manager",
        "[~] Refresh Desktop"
    };

    int32_t iy = cm_y + 8;
    int32_t mx = mouse_get_x();
    int32_t my = mouse_get_y();

    for (int i = 0; i < 5; i++) {
        bool hover = (mx >= cm_x && mx <= cm_x + (int32_t)cm_w && my >= iy && my < iy + 25);
        if (hover) {
            vbe_fill_rect(cm_x + 4, iy, cm_w - 8, 24, 0xFF14243A);
            vbe_draw_rect(cm_x + 4, iy, cm_w - 8, 24, COLOR_CYAN_GLOW);
        }
        font_draw_string(cm_x + 12, iy + 4, items[i], hover ? COLOR_CYAN_GLOW : COLOR_WHITE);
        iy += 26;
    }
}

/* ---------------- Action Center / Quick Settings ---------------- */

static void draw_quick_settings(vbe_screen_t* scr) {
    if (!g_quick_settings_open) return;

    uint32_t qs_w = 315;
    uint32_t qs_h = 300;
    uint32_t qs_x = scr->width - qs_w - 15;
    uint32_t qs_y = scr->height - 58 - qs_h;

    vbe_fill_rect(qs_x, qs_y, qs_w, qs_h, 0xFA0B111E);
    vbe_draw_glow_rect(qs_x, qs_y, qs_w, qs_h, COLOR_CYAN_GLOW, COLOR_CYAN_DARK);

    font_draw_string_glow(qs_x + 15, qs_y + 12, "Quick Settings", COLOR_WHITE, COLOR_CYAN_DARK);
    font_draw_string(qs_x + qs_w - 120, qs_y + 12, "[98% ⚡ AC Online]", COLOR_GREEN_OK);
    vbe_draw_line(qs_x, qs_y + 30, qs_x + qs_w, qs_y + 30, COLOR_BORDER_CYAN);

    const char* toggle_labels[] = { "Wi-Fi", "Bluetooth", "Airplane", "Night Light", "Battery Saver" };
    bool toggle_states[] = { g_qs_wifi, g_qs_bt, g_qs_airplane, g_qs_night_light, g_qs_battery_saver };

    for (int i = 0; i < 5; i++) {
        uint32_t col = i % 2;
        uint32_t row = i / 2;
        uint32_t sw_x = qs_x + 15 + (col * 145);
        uint32_t sw_y = qs_y + 40 + (row * 36);
        uint32_t sw_w = 135;
        uint32_t sw_h = 30;

        bool active = toggle_states[i];
        vbe_fill_rect(sw_x, sw_y, sw_w, sw_h, active ? 0xFF004455 : 0xFF0C1420);
        vbe_draw_rect(sw_x, sw_y, sw_w, sw_h, active ? COLOR_CYAN_GLOW : COLOR_DARK_GREY);

        uint32_t knob_x = active ? (sw_x + sw_w - 20) : (sw_x + 6);
        vbe_fill_circle(knob_x + 6, sw_y + 15, 6, active ? COLOR_CYAN_GLOW : COLOR_LIGHT_GREY);

        font_draw_string(sw_x + (active ? 8 : 22), sw_y + 7, toggle_labels[i], active ? COLOR_WHITE : COLOR_LIGHT_GREY);
    }

    uint32_t sy = qs_y + 160;
    vbe_draw_line(qs_x, sy - 8, qs_x + qs_w, sy - 8, COLOR_BORDER_CYAN);

    /* Volume */
    font_draw_string(qs_x + 15, sy, "Volume:", COLOR_WHITE);
    char vol_str[8];
    vol_str[0] = '0' + (g_qs_volume / 100);
    vol_str[1] = '0' + ((g_qs_volume % 100) / 10);
    vol_str[2] = '0' + (g_qs_volume % 10);
    vol_str[3] = '%'; vol_str[4] = '\0';
    font_draw_string(qs_x + 75, sy, vol_str, COLOR_CYAN_GLOW);

    uint32_t sl_track_w = 180;
    uint32_t sl_track_x = qs_x + 120;
    vbe_fill_rect(sl_track_x, sy + 3, sl_track_w, 8, 0xFF0B1424);
    vbe_draw_rect(sl_track_x, sy + 3, sl_track_w, 8, COLOR_BORDER_CYAN);
    uint32_t sl_fill_w = (sl_track_w * g_qs_volume) / 100;
    vbe_fill_rect(sl_track_x, sy + 3, sl_fill_w, 8, COLOR_CYAN_GLOW);
    vbe_fill_circle(sl_track_x + sl_fill_w, sy + 7, 6, COLOR_WHITE);

    /* Brightness */
    sy += 30;
    font_draw_string(qs_x + 15, sy, "Brightness:", COLOR_WHITE);
    char bri_str[8];
    bri_str[0] = '0' + (g_qs_brightness / 100);
    bri_str[1] = '0' + ((g_qs_brightness % 100) / 10);
    bri_str[2] = '0' + (g_qs_brightness % 10);
    bri_str[3] = '%'; bri_str[4] = '\0';
    font_draw_string(qs_x + 100, sy, bri_str, COLOR_AMBER_GLOW);

    uint32_t bri_track_x = qs_x + 145;
    uint32_t bri_track_w = 155;
    vbe_fill_rect(bri_track_x, sy + 3, bri_track_w, 8, 0xFF0B1424);
    vbe_draw_rect(bri_track_x, sy + 3, bri_track_w, 8, COLOR_BORDER_CYAN);
    uint32_t bri_fill_w = (bri_track_w * g_qs_brightness) / 100;
    vbe_fill_rect(bri_track_x, sy + 3, bri_fill_w, 8, COLOR_AMBER_GLOW);
    vbe_fill_circle(bri_track_x + bri_fill_w, sy + 7, 6, COLOR_WHITE);

    /* Power Buttons */
    sy += 40;
    vbe_draw_line(qs_x, sy - 8, qs_x + qs_w, sy - 8, COLOR_BORDER_CYAN);

    vbe_fill_rect(qs_x + 15, sy, 85, 24, 0xFF0D2137);
    vbe_draw_rect(qs_x + 15, sy, 85, 24, COLOR_BORDER_CYAN);
    font_draw_string(qs_x + 28, sy + 4, "[Sleep]", COLOR_CYAN_GLOW);

    vbe_fill_rect(qs_x + 115, sy, 85, 24, 0xFF14243A);
    vbe_draw_rect(qs_x + 115, sy, 85, 24, COLOR_CYAN_GLOW);
    font_draw_string(qs_x + 125, sy + 4, "[Restart]", COLOR_WHITE);

    vbe_fill_rect(qs_x + 215, sy, 85, 24, 0xFF44001A);
    vbe_draw_rect(qs_x + 215, sy, 85, 24, COLOR_CRIMSON);
    font_draw_string(qs_x + 223, sy + 4, "[Power]", COLOR_CRIMSON);
}

/* ---------------- Windows 11 Precision Aerodynamic White Cursor ---------------- */

static void draw_mouse_cursor(vbe_screen_t* scr, int32_t mx, int32_t my) {
    if (!scr || !scr->back_buffer) return;

    static const uint16_t shape_mask[19] = {
        0b1000000000000000,
        0b1100000000000000,
        0b1110000000000000,
        0b1111000000000000,
        0b1111100000000000,
        0b1111110000000000,
        0b1111111000000000,
        0b1111111100000000,
        0b1111111110000000,
        0b1111111111000000,
        0b1111110000000000,
        0b1101111000000000,
        0b1000111100000000,
        0b0000011110000000,
        0b0000001111000000,
        0b0000000111000000,
        0b0000000011000000,
        0b0000000001000000,
        0b0000000000000000
    };

    static const uint16_t border_mask[19] = {
        0b1000000000000000,
        0b1100000000000000,
        0b1010000000000000,
        0b1001000000000000,
        0b1000100000000000,
        0b1000010000000000,
        0b1000001000000000,
        0b1000000100000000,
        0b1000000010000000,
        0b1000000001000000,
        0b1000011111100000,
        0b1101001000000000,
        0b1011000100000000,
        0b0000100010000000,
        0b0000010001000000,
        0b0000001000100000,
        0b0000000100100000,
        0b0000000011000000,
        0b0000000000000000
    };

    /* Drop Shadow */
    for (int r = 0; r < 19; r++) {
        int py = my + r + 2;
        if (py < 0 || py >= (int)scr->height) continue;
        uint16_t row = shape_mask[r];
        for (int c = 0; c < 16; c++) {
            if ((row >> (15 - c)) & 1) {
                int px = mx + c + 2;
                if (px < 0 || px >= (int)scr->width) continue;
                vbe_putpixel(px, py, 0xFF020408);
            }
        }
    }

    /* Pure White Body + Dark Obsidian Border */
    for (int r = 0; r < 19; r++) {
        int py = my + r;
        if (py < 0 || py >= (int)scr->height) continue;
        uint16_t row_body   = shape_mask[r];
        uint16_t row_border = border_mask[r];

        for (int c = 0; c < 16; c++) {
            int px = mx + c;
            if (px < 0 || px >= (int)scr->width) continue;

            if ((row_border >> (15 - c)) & 1) {
                vbe_putpixel(px, py, 0xFF11141E);
            } else if ((row_body >> (15 - c)) & 1) {
                vbe_putpixel(px, py, 0xFFFFFFFF);
            }
        }
    }
}

/* ---------------- Interactive Mouse Event Dispatcher ---------------- */

void compositor_handle_mouse(int32_t mx, int32_t my, bool left, bool right) {
    bool left_clicked  = left && !s_prev_left;
    bool right_clicked = right && !s_prev_right;
    s_prev_left  = left;
    s_prev_right = right;

    vbe_screen_t* scr = vbe_get_screen();
    if (!scr) return;

    /* Window Dragging */
    if (left && s_drag_win_id != 0) {
        for (int i = 0; i < MAX_WINDOWS; i++) {
            if (g_windows[i].active && g_windows[i].id == s_drag_win_id) {
                g_windows[i].x = mx - s_drag_ox;
                g_windows[i].y = my - s_drag_oy;
                break;
            }
        }
        return;
    } else if (!left) {
        s_drag_win_id = 0;
    }

    /* 1. Explicit Right Click Context Menu */
    if (right_clicked) {
        g_context_menu_open = true;
        g_context_x = mx;
        g_context_y = my;
        g_quick_settings_open = false;
        g_start_menu_open = false;
        return;
    }

    if (!left_clicked) return;

    /* 2. Check Context Menu Click */
    if (g_context_menu_open) {
        uint32_t cm_w = 195;
        uint32_t cm_h = 145;
        int32_t  cm_x = g_context_x;
        int32_t  cm_y = g_context_y;
        if (cm_x + (int32_t)cm_w > (int32_t)scr->width) cm_x = scr->width - cm_w - 10;
        if (cm_y + (int32_t)cm_h > (int32_t)scr->height - 50) cm_y = scr->height - 50 - cm_h;

        if (mx >= cm_x && mx <= cm_x + (int32_t)cm_w && my >= cm_y && my <= cm_y + (int32_t)cm_h) {
            int item = (my - (cm_y + 8)) / 26;
            if (item == 0) compositor_set_wallpaper((g_active_wallpaper + 1) % 4);
            else if (item == 1) app_launch_hub();
            else if (item == 2) app_launch_terminal();
            else if (item == 3) app_launch_taskmgr();
            else if (item == 4) { /* Refresh */ }
        }
        g_context_menu_open = false;
        return;
    }

    /* 3. Check Quick Settings Panel Click */
    if (g_quick_settings_open) {
        uint32_t qs_w = 315;
        uint32_t qs_h = 300;
        uint32_t qs_x = scr->width - qs_w - 15;
        uint32_t qs_y = scr->height - 58 - qs_h;

        if (mx >= (int32_t)qs_x && mx <= (int32_t)(qs_x + qs_w) &&
            my >= (int32_t)qs_y && my <= (int32_t)(qs_y + qs_h)) {

            for (int i = 0; i < 5; i++) {
                uint32_t col = i % 2;
                uint32_t row = i / 2;
                uint32_t sw_x = qs_x + 15 + (col * 145);
                uint32_t sw_y = qs_y + 40 + (row * 36);
                if (mx >= (int32_t)sw_x && mx <= (int32_t)(sw_x + 135) &&
                    my >= (int32_t)sw_y && my <= (int32_t)(sw_y + 30)) {
                    if (i == 0) g_qs_wifi = !g_qs_wifi;
                    else if (i == 1) g_qs_bt = !g_qs_bt;
                    else if (i == 2) g_qs_airplane = !g_qs_airplane;
                    else if (i == 3) g_qs_night_light = !g_qs_night_light;
                    else if (i == 4) g_qs_battery_saver = !g_qs_battery_saver;
                    return;
                }
            }

            uint32_t sl_track_x = qs_x + 120;
            uint32_t sl_track_w = 180;
            if (mx >= (int32_t)sl_track_x && mx <= (int32_t)(sl_track_x + sl_track_w) &&
                my >= (int32_t)(qs_y + 155) && my <= (int32_t)(qs_y + 185)) {
                g_qs_volume = ((mx - sl_track_x) * 100) / sl_track_w;
                if (g_qs_volume > 100) g_qs_volume = 100;
                return;
            }

            uint32_t bri_track_x = qs_x + 145;
            uint32_t bri_track_w = 155;
            if (mx >= (int32_t)bri_track_x && mx <= (int32_t)(bri_track_x + bri_track_w) &&
                my >= (int32_t)(qs_y + 185) && my <= (int32_t)(qs_y + 215)) {
                g_qs_brightness = ((mx - bri_track_x) * 100) / bri_track_w;
                if (g_qs_brightness > 100) g_qs_brightness = 100;
                return;
            }

            uint32_t py = qs_y + 240;
            if (my >= (int32_t)py && my <= (int32_t)(py + 24)) {
                if (mx >= (int32_t)(qs_x + 15) && mx <= (int32_t)(qs_x + 100)) {
                    acpi_enter_s0ix_standby();
                } else if (mx >= (int32_t)(qs_x + 115) && mx <= (int32_t)(qs_x + 200)) {
                    acpi_reboot();
                } else if (mx >= (int32_t)(qs_x + 215) && mx <= (int32_t)(qs_x + 300)) {
                    acpi_poweroff();
                }
                return;
            }
            return;
        } else {
            g_quick_settings_open = false;
        }
    }

    /* 4. Check Window Clicks (Topmost down) */
    for (int i = MAX_WINDOWS - 1; i >= 0; i--) {
        window_t* win = &g_windows[i];
        if (!win->active || win->minimized) continue;

        /* Titlebar click */
        if (mx >= (int32_t)win->x && mx <= (int32_t)(win->x + win->w) &&
            my >= (int32_t)win->y && my <= (int32_t)(win->y + 28)) {

            /* Close Button */
            int32_t btn_x = win->x + win->w - 65;
            if (mx >= btn_x + 40 && mx <= btn_x + 62) {
                compositor_close_window(win->id);
                return;
            }

            /* Minimize Button */
            if (mx >= btn_x && mx <= btn_x + 18) {
                win->minimized = true;
                return;
            }

            /* Drag Titlebar */
            compositor_set_active_window(win->id);
            s_drag_win_id = win->id;
            s_drag_ox = mx - win->x;
            s_drag_oy = my - win->y;
            return;
        }

        /* Body click */
        if (mx >= (int32_t)win->x && mx <= (int32_t)(win->x + win->w) &&
            my >= (int32_t)(win->y + 28) && my <= (int32_t)(win->y + win->h)) {
            compositor_set_active_window(win->id);

            /* Interactive Content Handlers */
            if (kstrncmp(win->title, "Airlys Sovereign Hub", 20) == 0) {
                app_hub_click(win, mx, my);
            } else if (kstrcmp(win->title, "airlys-files") == 0) {
                app_files_click(win, mx, my);
            }
            return;
        }
    }

    /* 5. Check Desktop Tile Clicks (This PC, Files, Task Manager, Search, Settings) */
    uint32_t diy = 30;
    for (int i = 0; i < 5; i++) {
        uint32_t dix = 25;
        if (mx >= (int32_t)dix && mx <= (int32_t)(dix + 56) &&
            my >= (int32_t)diy && my <= (int32_t)(diy + 70)) {
            if (i == 0) app_launch_hub();
            else if (i == 1) app_launch_files();
            else if (i == 2) app_launch_taskmgr();
            else if (i == 3) app_launch_search();
            else if (i == 4) app_launch_settings();
            return;
        }
        diy += 84;
    }

    /* 6. Check Taskbar Clicks */
    uint32_t bar_h = 48;
    uint32_t bar_y = scr->height - bar_h;

    if (my >= (int32_t)bar_y && my <= (int32_t)scr->height) {
        /* Start Button */
        if (mx >= 14 && mx <= 46) {
            compositor_toggle_start_menu();
            return;
        }

        /* Search Pill */
        if (mx >= 56 && mx <= 266) {
            app_launch_search();
            return;
        }

        /* Centered App Icons */
        uint32_t app_center_w = 7 * 54;
        uint32_t app_x = (scr->width / 2) - (app_center_w / 2);

        for (int i = 0; i < 7; i++) {
            uint32_t tx = app_x + (i * 54);
            if (mx >= (int32_t)tx && mx <= (int32_t)(tx + 36)) {
                if (i == 0) app_launch_terminal();
                else if (i == 1) app_launch_files();
                else if (i == 2) app_launch_code();
                else if (i == 4) app_launch_taskmgr();
                else if (i == 5) app_launch_search();
                else if (i == 6) app_launch_hub();
                return;
            }
        }

        /* System Tray Click */
        uint32_t tray_w = 260;
        uint32_t tray_x = scr->width - tray_w - 10;
        if (mx >= (int32_t)tray_x && mx <= (int32_t)(tray_x + tray_w)) {
            compositor_toggle_quick_settings();
            return;
        }
    }

    /* Click on empty desktop closes Start Menu & Quick Settings */
    if (g_start_menu_open) g_start_menu_open = false;
    if (g_quick_settings_open) g_quick_settings_open = false;
}

/* ---------------- Render Frame Pipeline ---------------- */

void compositor_render_frame(void) {
    vbe_screen_t* scr = vbe_get_screen();
    if (!scr || !scr->initialized) return;

    /* 1. Lock Screen check */
    if (g_lock_screen_open) {
        app_lockscreen_draw();
        draw_mouse_cursor(scr, mouse_get_x(), mouse_get_y());
        vbe_swap_buffers();
        return;
    }

    /* 2. Full-Frame Edge-to-Edge Desktop Wallpaper */
    draw_desktop_wallpaper(scr);

    /* 3. Desktop Quick Icons (LVGL Grid Tiles) */
    draw_desktop_icons(scr);

    /* 4. Inactive Windows */
    for (int i = 0; i < MAX_WINDOWS; i++) {
        if (g_windows[i].active && g_windows[i].id != g_active_window_id) {
            draw_window(&g_windows[i]);
        }
    }
    /* Active Window on Top */
    for (int i = 0; i < MAX_WINDOWS; i++) {
        if (g_windows[i].active && g_windows[i].id == g_active_window_id) {
            draw_window(&g_windows[i]);
            break;
        }
    }

    /* 5. Full-Width Taskbar */
    draw_taskbar(scr);

    /* 6. Overlays (Start Menu, Quick Settings, Context Menu) */
    draw_start_menu(scr);
    draw_quick_settings(scr);
    draw_context_menu(scr);

    /* 7. Precision Aerodynamic White Cursor Overlay */
    draw_mouse_cursor(scr, mouse_get_x(), mouse_get_y());

    /* 8. Push to Screen */
    vbe_swap_buffers();
}
