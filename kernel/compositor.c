#include <airlys/compositor.h>
#include <airlys/apps.h>
#include <airlys/vbe.h>
#include <airlys/font.h>
#include <airlys/string.h>
#include <airlys/rtc.h>
#include <airlys/printk.h>
#include <airlys/mouse.h>
#include <airlys/wallpaper_logo.h>

static window_t g_windows[MAX_WINDOWS];
static uint32_t g_num_windows = 0;
static uint32_t g_active_window_id = 0;
static bool     g_start_menu_open = false;
static bool     g_lock_screen_open = false;

/* Mouse Interaction State */
static bool     s_prev_left = false;
static uint32_t s_drag_win_id = 0;
static int32_t  s_drag_ox = 0;
static int32_t  s_drag_oy = 0;

void compositor_init(void) {
    g_num_windows = 0;
    g_active_window_id = 0;
    g_start_menu_open = false;
    g_lock_screen_open = false;
    kmemset(g_windows, 0, sizeof(g_windows));

    /* Initialize VBE 1024x768 @ 32bpp Framebuffer (or keep active hardware GOP) */
    vbe_init(1024, 768);
    vbe_allocate_backbuffer();
    apps_init();

    /* Clean desktop startup: keep windows ready on dock without obstructing screen */
    printk(KERN_INFO "compositor: Phase 7 Desktop Window Compositor & Acrylic Shell online\n");
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
                for (int j = 0; j < MAX_WINDOWS; j++) {
                    if (g_windows[j].active && !g_windows[j].minimized) {
                        g_active_window_id = g_windows[j].id;
                        break;
                    }
                }
            }
            return;
        }
    }
}

void compositor_set_active_window(uint32_t id) {
    g_active_window_id = id;
}

void compositor_toggle_start_menu(void) {
    g_start_menu_open = !g_start_menu_open;
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

/* ---------------- Graphical Vector Icons for Taskbar Dock ---------------- */

static void draw_icon_start(uint32_t x, uint32_t y) {
    /* Outer cyan glow ring */
    vbe_draw_circle(x + 14, y + 14, 14, COLOR_CYAN_GLOW);
    vbe_draw_circle(x + 14, y + 14, 13, 0xFF00384D);
    vbe_fill_circle(x + 14, y + 14, 12, 0xFF031926);

    /* Stylized Eagle Crest / 'A' Glyph */
    vbe_draw_line(x + 14, y + 6, x + 8, y + 20, COLOR_CYAN_GLOW);
    vbe_draw_line(x + 14, y + 6, x + 20, y + 20, COLOR_CYAN_GLOW);
    vbe_draw_line(x + 10, y + 16, x + 18, y + 16, COLOR_CYAN_GLOW);
    vbe_draw_line(x + 7, y + 13, x + 14, y + 9, 0xFF00C853);
    vbe_draw_line(x + 21, y + 13, x + 14, y + 9, 0xFF00C853);
}

static void draw_icon_terminal(uint32_t x, uint32_t y) {
    vbe_fill_rect(x + 3, y + 3, 26, 22, 0xFF0B1424);
    vbe_draw_rect(x + 3, y + 3, 26, 22, COLOR_BORDER_CYAN);
    /* Prompt > */
    vbe_draw_line(x + 8, y + 9, x + 13, y + 14, COLOR_CYAN_GLOW);
    vbe_draw_line(x + 13, y + 14, x + 8, y + 19, COLOR_CYAN_GLOW);
    /* Block cursor _ */
    vbe_fill_rect(x + 16, y + 17, 7, 3, COLOR_CYAN_GLOW);
}

static void draw_icon_files(uint32_t x, uint32_t y) {
    vbe_fill_rect(x + 3, y + 3, 26, 22, 0xFF081C30);
    vbe_draw_rect(x + 3, y + 3, 26, 22, COLOR_BORDER_CYAN);
    /* Folder tab */
    vbe_fill_rect(x + 6, y + 7, 9, 3, 0xFF0284C7);
    /* Folder back */
    vbe_fill_rect(x + 6, y + 10, 20, 12, 0xFF0369A1);
    /* Folder front flap */
    vbe_fill_rect(x + 6, y + 13, 20, 9, 0xFF0284C7);
    vbe_draw_line(x + 6, y + 13, x + 26, y + 13, COLOR_CYAN_GLOW);
}

static void draw_icon_code(uint32_t x, uint32_t y) {
    vbe_fill_rect(x + 3, y + 3, 26, 22, 0xFF141332);
    vbe_draw_rect(x + 3, y + 3, 26, 22, COLOR_BORDER_CYAN);
    /* < */
    vbe_draw_line(x + 9, y + 9, x + 6, y + 14, COLOR_CYAN_GLOW);
    vbe_draw_line(x + 6, y + 14, x + 9, y + 19, COLOR_CYAN_GLOW);
    /* / */
    vbe_draw_line(x + 18, y + 7, x + 14, y + 21, 0xFFFFB300);
    /* > */
    vbe_draw_line(x + 23, y + 9, x + 26, y + 14, COLOR_CYAN_GLOW);
    vbe_draw_line(x + 26, y + 14, x + 23, y + 19, COLOR_CYAN_GLOW);
}

static void draw_icon_store(uint32_t x, uint32_t y) {
    vbe_fill_rect(x + 3, y + 3, 26, 22, 0xFF0C2226);
    vbe_draw_rect(x + 3, y + 3, 26, 22, COLOR_BORDER_CYAN);
    /* Shopping bag body */
    vbe_fill_rect(x + 7, y + 11, 18, 12, 0xFF0D9488);
    /* Bag handle */
    vbe_draw_line(x + 12, y + 8, x + 14, y + 6, COLOR_CYAN_GLOW);
    vbe_draw_line(x + 14, y + 6, x + 18, y + 6, COLOR_CYAN_GLOW);
    vbe_draw_line(x + 18, y + 6, x + 20, y + 8, COLOR_CYAN_GLOW);
    /* Center gem / diamond */
    vbe_fill_circle(x + 16, y + 16, 2, 0xFFFF007F);
}

static void draw_icon_taskmgr(uint32_t x, uint32_t y) {
    vbe_fill_rect(x + 3, y + 3, 26, 22, 0xFF071924);
    vbe_draw_rect(x + 3, y + 3, 26, 22, COLOR_BORDER_CYAN);
    /* Pulse heartbeat graph */
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
    /* Magnifying glass */
    vbe_draw_circle(x + 13, y + 12, 5, COLOR_CYAN_GLOW);
    vbe_draw_line(x + 17, y + 16, x + 23, y + 21, COLOR_CYAN_GLOW);
    vbe_draw_line(x + 18, y + 16, x + 24, y + 21, COLOR_CYAN_GLOW);
}

static void draw_icon_settings(uint32_t x, uint32_t y) {
    vbe_fill_rect(x + 3, y + 3, 26, 22, 0xFF161F2E);
    vbe_draw_rect(x + 3, y + 3, 26, 22, COLOR_BORDER_CYAN);
    /* Cyber gear hub */
    vbe_draw_circle(x + 16, y + 14, 4, COLOR_CYAN_GLOW);
    /* Spokes */
    vbe_draw_line(x + 16, y + 7, x + 16, y + 10, COLOR_CYAN_GLOW);
    vbe_draw_line(x + 16, y + 18, x + 16, y + 21, COLOR_CYAN_GLOW);
    vbe_draw_line(x + 9, y + 14, x + 12, y + 14, COLOR_CYAN_GLOW);
    vbe_draw_line(x + 20, y + 14, x + 23, y + 14, COLOR_CYAN_GLOW);
}

/* ---------------- Desktop Wallpaper & Logo ---------------- */

static void draw_desktop_wallpaper(vbe_screen_t* scr) {
    /* 1. Deep matte cyber obsidian background */
    vbe_fill_rect(0, 0, scr->width, scr->height, 0xFF060911);

    /* 2. Center the exact Airlys Logo created by Er. Adam Eehan */
    int32_t lx = ((int32_t)scr->width - WALLPAPER_LOGO_W) / 2;
    int32_t ly = ((int32_t)scr->height - WALLPAPER_LOGO_H) / 2 - 40;

    for (int y = 0; y < WALLPAPER_LOGO_H; y++) {
        int py = ly + y;
        if (py < 0 || py >= (int)scr->height) continue;
        uint32_t row_offset = y * WALLPAPER_LOGO_W;
        for (int x = 0; x < WALLPAPER_LOGO_W; x++) {
            int px = lx + x;
            if (px < 0 || px >= (int)scr->width) continue;
            uint32_t color = g_wallpaper_logo_pixels[row_offset + x];
            vbe_putpixel(px, py, color);
        }
    }

    /* 3. Ambient neon glow around the logo */
    uint32_t cx = scr->width / 2;
    uint32_t cy = (scr->height / 2) - 40;
    vbe_draw_circle(cx, cy, 210, 0x1500F2FE);
    vbe_draw_circle(cx, cy, 211, 0x0D00F2FE);

    /* 4. Sovereign Brand Title below logo */
    const char* brand = "AIRLYS SOVEREIGN OPERATING SYSTEM";
    const char* sub   = "Engineered by Er. Adam Eehan & Sovereign AI Aira";
    font_draw_string(cx - (kstrlen(brand) * 4), cy + 125, brand, COLOR_CYAN_GLOW);
    font_draw_string(cx - (kstrlen(sub) * 4), cy + 145, sub, COLOR_LIGHT_GREY);
}

/* ---------------- Window Drawing ---------------- */

static void draw_window(window_t* win) {
    if (!win->active || win->minimized) return;

    /* 1. Frosted Acrylic Translucent Window Body */
    vbe_fill_rect(win->x, win->y, win->w, win->h, COLOR_FROSTED_GLASS);

    /* 2. Window Titlebar */
    uint32_t tb_color = (win->id == g_active_window_id) ? COLOR_TITLEBAR_BG : 0xEE090E18;
    vbe_fill_rect(win->x, win->y, win->w, 26, tb_color);

    /* Titlebar 1px separator */
    vbe_draw_line(win->x, win->y + 26, win->x + win->w, win->y + 26, COLOR_BORDER_CYAN);

    /* Title string */
    font_draw_string(win->x + 10, win->y + 5, win->title,
                     (win->id == g_active_window_id) ? COLOR_CYAN_GLOW : COLOR_LIGHT_GREY);

    /* 3. Window Control Buttons (— □ ✕) */
    uint32_t btn_x = win->x + win->w - 60;
    font_draw_string(btn_x, win->y + 5, "-", COLOR_LIGHT_GREY);
    font_draw_string(btn_x + 18, win->y + 5, "[ ]", COLOR_LIGHT_GREY);

    /* Close button with subtle crimson accent */
    vbe_fill_rect(btn_x + 38, win->y + 3, 18, 20, 0x33FF3366);
    font_draw_string(btn_x + 43, win->y + 5, "x", COLOR_CRIMSON);

    /* 4. Window Content Area */
    if (win->draw_content) {
        win->draw_content(win);
    }

    /* 5. Outer 1px Electric Cyan Glow Border */
    uint32_t border_color = (win->id == g_active_window_id) ? COLOR_CYAN_GLOW : COLOR_BORDER_CYAN;
    vbe_draw_glow_rect(win->x, win->y, win->w, win->h, border_color, COLOR_CYAN_DARK);
}

/* ---------------- Centered Floating Taskbar Dock ---------------- */

static void draw_taskbar(vbe_screen_t* scr) {
    uint32_t bar_h = 44;
    uint32_t bar_y = scr->height - bar_h - 10;
    uint32_t dock_w = 480;
    uint32_t dock_x = (scr->width / 2) - (dock_w / 2);

    /* Centered Floating Frosted Glass Dock */
    vbe_fill_rect(dock_x, bar_y, dock_w, bar_h, COLOR_TASKBAR_BG);
    vbe_draw_glow_rect(dock_x, bar_y, dock_w, bar_h, COLOR_CYAN_GLOW, COLOR_CYAN_DARK);

    /* Start Button with Eagle Crest */
    uint32_t sx = dock_x + 12;
    uint32_t sy = bar_y + 8;
    draw_icon_start(sx, sy);

    /* Pinned App Icons (Vector Graphic Glyphs) */
    uint32_t app_x = sx + 44;
    for (int i = 0; i < 7; i++) {
        uint32_t tx = app_x + (i * 54);
        uint32_t ty = bar_y + 8;

        if (i == 0) draw_icon_terminal(tx, ty);
        else if (i == 1) draw_icon_files(tx, ty);
        else if (i == 2) draw_icon_code(tx, ty);
        else if (i == 3) draw_icon_store(tx, ty);
        else if (i == 4) draw_icon_taskmgr(tx, ty);
        else if (i == 5) draw_icon_search(tx, ty);
        else if (i == 6) draw_icon_settings(tx, ty);

        /* Indicator pill if app is running */
        bool is_open = false;
        bool is_active = false;
        for (int w = 0; w < MAX_WINDOWS; w++) {
            if (!g_windows[w].active) continue;
            if ((i == 0 && kstrcmp(g_windows[w].title, "airlys-term") == 0) ||
                (i == 1 && kstrcmp(g_windows[w].title, "airlys-files") == 0) ||
                (i == 4 && kstrcmp(g_windows[w].title, "Aira Task Manager") == 0) ||
                (i == 5 && kstrncmp(g_windows[w].title, "Airlys", 6) == 0)) {
                is_open = true;
                if (g_windows[w].id == g_active_window_id && !g_windows[w].minimized) {
                    is_active = true;
                }
                break;
            }
        }

        if (is_active) {
            vbe_fill_rect(tx + 10, ty + 28, 12, 2, COLOR_CYAN_GLOW);
        } else if (is_open) {
            vbe_fill_rect(tx + 12, ty + 28, 8, 2, COLOR_LIGHT_GREY);
        }
    }

    /* Right System Tray (Clock, Link, Volume, Shield) */
    uint32_t tray_x = scr->width - 190;
    vbe_fill_rect(tray_x, bar_y, 180, bar_h, COLOR_TASKBAR_BG);
    vbe_draw_glow_rect(tray_x, bar_y, 180, bar_h, COLOR_BORDER_CYAN, COLOR_CYAN_DARK);

    rtc_time_t t = rtc_get_time();
    char clk_buf[16];
    clk_buf[0] = '0' + (t.hour / 10);
    clk_buf[1] = '0' + (t.hour % 10);
    clk_buf[2] = ':';
    clk_buf[3] = '0' + (t.minute / 10);
    clk_buf[4] = '0' + (t.minute % 10);
    clk_buf[5] = '\0';

    font_draw_string(tray_x + 12, bar_y + 14, clk_buf, COLOR_CYAN_GLOW);
    font_draw_string(tray_x + 60, bar_y + 14, "[eth0] [80%]", COLOR_LIGHT_GREY);
}

/* ---------------- Start Menu ---------------- */

static void draw_start_menu(vbe_screen_t* scr) {
    if (!g_start_menu_open) return;

    uint32_t sm_w = 440;
    uint32_t sm_h = 420;
    uint32_t sm_x = (scr->width / 2) - (sm_w / 2);
    uint32_t sm_y = scr->height - 60 - sm_h;

    /* Frosted Acrylic Card */
    vbe_fill_rect(sm_x, sm_y, sm_w, sm_h, 0xFA0B111E);
    vbe_draw_glow_rect(sm_x, sm_y, sm_w, sm_h, COLOR_CYAN_GLOW, COLOR_CYAN_DARK);

    /* Top Search Bar */
    uint32_t sb_x = sm_x + 20;
    uint32_t sb_y = sm_y + 18;
    vbe_fill_rect(sb_x, sb_y, sm_w - 40, 30, 0xFF070B14);
    vbe_draw_rect(sb_x, sb_y, sm_w - 40, 30, COLOR_BORDER_CYAN);
    font_draw_string(sb_x + 12, sb_y + 7, "Search apps, files, or ask Aira AI...", COLOR_LIGHT_GREY);

    /* Pinned Apps Header */
    uint32_t py = sm_y + 60;
    font_draw_string(sm_x + 20, py, "Pinned Sovereign Applications", COLOR_CYAN_GLOW);
    py += 22;

    const char* pinned[] = {
        "airlys-term", "airlys-files", "aira-code", "airastore",
        "aira-taskmgr", "aira-sec", "aira-settings", "aira-calc"
    };

    for (int r = 0; r < 2; r++) {
        for (int c = 0; c < 4; c++) {
            uint32_t ax = sm_x + 20 + (c * 100);
            uint32_t ay = py + (r * 40);
            vbe_fill_rect(ax, ay, 90, 32, 0xFF0E1626);
            vbe_draw_rect(ax, ay, 90, 32, COLOR_BORDER_CYAN);
            font_draw_string(ax + 6, ay + 8, pinned[r * 4 + c], COLOR_WHITE);
        }
    }

    /* Recommended Section */
    uint32_t ry = py + 95;
    font_draw_string(sm_x + 20, ry, "Recommended & Recent Files", COLOR_CYAN_GLOW);
    ry += 20;

    const char* recents[] = {
        "[PKG] demo.apkg     - Aira Package (Installed 2m ago)",
        "[SRC] system.aira   - Sovereign Script (Edited today)",
        "[DOC] covenant.txt  - The Open-Source Covenant"
    };

    for (int i = 0; i < 3; i++) {
        vbe_fill_rect(sm_x + 20, ry, sm_w - 40, 22, 0xFF090E1A);
        font_draw_string(sm_x + 28, ry + 3, recents[i], COLOR_LIGHT_GREY);
        ry += 26;
    }

    /* Bottom User & Power Bar */
    uint32_t uy = sm_y + sm_h - 38;
    vbe_fill_rect(sm_x, uy, sm_w, 38, 0xFF070B14);
    vbe_draw_line(sm_x, uy, sm_x + sm_w, uy, COLOR_BORDER_CYAN);

    /* User Avatar & Name */
    vbe_draw_circle(sm_x + 25, uy + 19, 12, COLOR_CYAN_GLOW);
    font_draw_char(sm_x + 22, uy + 11, 'A', COLOR_CYAN_GLOW);
    font_draw_string(sm_x + 45, uy + 6, "Er. Adam Eehan", COLOR_WHITE);
    font_draw_string(sm_x + 45, uy + 20, "Founder & CEO, Aira Group", COLOR_GREEN_OK);

    /* Power Button Glyph */
    font_draw_string(sm_x + sm_w - 70, uy + 11, "[Power]", COLOR_CRIMSON);
}

/* ---------------- Mouse Pointer Cursor ---------------- */

static void draw_mouse_cursor(vbe_screen_t* scr, int32_t mx, int32_t my) {
    if (!scr || !scr->back_buffer) return;

    /* 14x20 Futuristic Cyber Neon Arrow Cursor */
    static const uint16_t cursor_mask[20] = {
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
        0b0000000000000000,
        0b0000000000000000
    };

    static const uint16_t cursor_border[20] = {
        0b1100000000000000,
        0b1010000000000000,
        0b1001000000000000,
        0b1000100000000000,
        0b1000010000000000,
        0b1000001000000000,
        0b1000000100000000,
        0b1000000010000000,
        0b1000000001000000,
        0b1000000000100000,
        0b1000111000000000,
        0b1110100100000000,
        0b0100010010000000,
        0b0000001001000000,
        0b0000000100100000,
        0b0000000010010000,
        0b0000000001010000,
        0b0000000000110000,
        0b0000000000000000,
        0b0000000000000000
    };

    for (int r = 0; r < 18; r++) {
        int py = my + r;
        if (py < 0 || py >= (int)scr->height) continue;
        uint16_t row_fill   = cursor_mask[r];
        uint16_t row_border = cursor_border[r];

        for (int c = 0; c < 16; c++) {
            int px = mx + c;
            if (px < 0 || px >= (int)scr->width) continue;

            if ((row_border >> (15 - c)) & 1) {
                vbe_putpixel(px, py, COLOR_CYAN_GLOW);
            } else if ((row_fill >> (15 - c)) & 1) {
                vbe_putpixel(px, py, 0xFFFFFFFF);
            }
        }
    }
}

/* ---------------- Interactive Mouse Event Dispatcher ---------------- */

void compositor_handle_mouse(int32_t mx, int32_t my, bool left, bool right) {
    (void)right;
    bool clicked = left && !s_prev_left;
    s_prev_left = left;

    /* Handle Window Dragging */
    if (left && s_drag_win_id != 0) {
        for (int i = 0; i < MAX_WINDOWS; i++) {
            if (g_windows[i].active && g_windows[i].id == s_drag_win_id) {
                g_windows[i].x = mx - s_drag_ox;
                g_windows[i].y = my - s_drag_oy;
                break;
            }
        }
    } else if (!left) {
        s_drag_win_id = 0;
    }

    if (!clicked) return;

    vbe_screen_t* scr = vbe_get_screen();
    if (!scr) return;

    /* 1. Check Window Clicks (Topmost down) */
    for (int i = MAX_WINDOWS - 1; i >= 0; i--) {
        window_t* win = &g_windows[i];
        if (!win->active || win->minimized) continue;

        /* Check Titlebar click */
        if (mx >= (int32_t)win->x && mx <= (int32_t)(win->x + win->w) &&
            my >= (int32_t)win->y && my <= (int32_t)(win->y + 26)) {

            /* Close Button [x] */
            int32_t btn_x = win->x + win->w - 60;
            if (mx >= btn_x + 36 && mx <= btn_x + 58) {
                compositor_close_window(win->id);
                return;
            }

            /* Minimize Button [-] */
            if (mx >= btn_x && mx <= btn_x + 18) {
                win->minimized = true;
                return;
            }

            /* Titlebar drag */
            compositor_set_active_window(win->id);
            s_drag_win_id = win->id;
            s_drag_ox = mx - win->x;
            s_drag_oy = my - win->y;
            return;
        }

        /* Check Window Body click to focus */
        if (mx >= (int32_t)win->x && mx <= (int32_t)(win->x + win->w) &&
            my >= (int32_t)win->y && my <= (int32_t)(win->y + win->h)) {
            compositor_set_active_window(win->id);
            return;
        }
    }

    /* 2. Check Taskbar Dock Clicks */
    uint32_t bar_h = 44;
    uint32_t bar_y = scr->height - bar_h - 10;
    uint32_t dock_w = 480;
    uint32_t dock_x = (scr->width / 2) - (dock_w / 2);

    if (my >= (int32_t)bar_y && my <= (int32_t)(bar_y + bar_h)) {
        /* Start Button */
        uint32_t sx = dock_x + 12;
        if (mx >= (int32_t)sx && mx <= (int32_t)(sx + 32)) {
            compositor_toggle_start_menu();
            return;
        }

        /* Pinned App Slots */
        uint32_t app_x = sx + 44;
        for (int i = 0; i < 7; i++) {
            uint32_t tx = app_x + (i * 54);
            if (mx >= (int32_t)tx && mx <= (int32_t)(tx + 36)) {
                if (i == 0) {
                    /* Terminal */
                    bool found = false;
                    for (int w = 0; w < MAX_WINDOWS; w++) {
                        if (g_windows[w].active && kstrcmp(g_windows[w].title, "airlys-term") == 0) {
                            g_windows[w].minimized = !g_windows[w].minimized;
                            if (!g_windows[w].minimized) compositor_set_active_window(g_windows[w].id);
                            found = true;
                            break;
                        }
                    }
                    if (!found) app_launch_terminal();
                } else if (i == 1) {
                    /* Files */
                    bool found = false;
                    for (int w = 0; w < MAX_WINDOWS; w++) {
                        if (g_windows[w].active && kstrcmp(g_windows[w].title, "airlys-files") == 0) {
                            g_windows[w].minimized = !g_windows[w].minimized;
                            if (!g_windows[w].minimized) compositor_set_active_window(g_windows[w].id);
                            found = true;
                            break;
                        }
                    }
                    if (!found) app_launch_files();
                } else if (i == 4) {
                    /* Task Manager */
                    bool found = false;
                    for (int w = 0; w < MAX_WINDOWS; w++) {
                        if (g_windows[w].active && kstrcmp(g_windows[w].title, "Aira Task Manager") == 0) {
                            g_windows[w].minimized = !g_windows[w].minimized;
                            if (!g_windows[w].minimized) compositor_set_active_window(g_windows[w].id);
                            found = true;
                            break;
                        }
                    }
                    if (!found) app_launch_taskmgr();
                } else if (i == 5) {
                    /* Search */
                    bool found = false;
                    for (int w = 0; w < MAX_WINDOWS; w++) {
                        if (g_windows[w].active && kstrncmp(g_windows[w].title, "Airlys", 6) == 0) {
                            g_windows[w].minimized = !g_windows[w].minimized;
                            if (!g_windows[w].minimized) compositor_set_active_window(g_windows[w].id);
                            found = true;
                            break;
                        }
                    }
                    if (!found) app_launch_search();
                }
                return;
            }
        }
    }
}

/* ---------------- Render Frame Pipeline ---------------- */

void compositor_render_frame(void) {
    vbe_screen_t* scr = vbe_get_screen();
    if (!scr || !scr->initialized) return;

    /* 1. If Lock Screen active, draw lock screen and exit */
    if (g_lock_screen_open) {
        app_lockscreen_draw();
        draw_mouse_cursor(scr, mouse_get_x(), mouse_get_y());
        vbe_swap_buffers();
        return;
    }

    /* 2. Desktop Wallpaper & Sovereign Logo */
    draw_desktop_wallpaper(scr);

    /* 3. Render Inactive Windows */
    for (int i = 0; i < MAX_WINDOWS; i++) {
        if (g_windows[i].active && g_windows[i].id != g_active_window_id) {
            draw_window(&g_windows[i]);
        }
    }
    /* Render Active Window on Top */
    for (int i = 0; i < MAX_WINDOWS; i++) {
        if (g_windows[i].active && g_windows[i].id == g_active_window_id) {
            draw_window(&g_windows[i]);
            break;
        }
    }

    /* 4. Taskbar Dock */
    draw_taskbar(scr);

    /* 5. Start Menu Overlay */
    draw_start_menu(scr);

    /* 6. Mouse Cursor Overlay */
    draw_mouse_cursor(scr, mouse_get_x(), mouse_get_y());

    /* 7. Push to Screen */
    vbe_swap_buffers();
}
