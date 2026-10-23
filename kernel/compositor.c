#include <airlys/compositor.h>
#include <airlys/apps.h>
#include <airlys/vbe.h>
#include <airlys/font.h>
#include <airlys/string.h>
#include <airlys/rtc.h>
#include <airlys/printk.h>

static window_t g_windows[MAX_WINDOWS];
static uint32_t g_num_windows = 0;
static uint32_t g_active_window_id = 0;
static bool     g_start_menu_open = false;
static bool     g_lock_screen_open = false;

void compositor_init(void) {
    g_num_windows = 0;
    g_active_window_id = 0;
    g_start_menu_open = false;
    g_lock_screen_open = false;
    kmemset(g_windows, 0, sizeof(g_windows));

    /* Initialize VBE 1024x768 @ 32bpp Framebuffer */
    vbe_init(1024, 768);
    vbe_allocate_backbuffer();
    apps_init();

    /* Pre-seed core windows */
    app_launch_terminal();
    app_launch_taskmgr();

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

static void draw_desktop_wallpaper(vbe_screen_t* scr) {
    /* Deep matte obsidian wallpaper */
    vbe_fill_rect(0, 0, scr->width, scr->height, COLOR_OBSIDIAN);

    /* Subtle glowing eagle watermark emblem in center */
    uint32_t cx = scr->width / 2;
    uint32_t cy = (scr->height / 2) - 20;

    vbe_draw_circle(cx, cy, 140, 0x1A00F2FE);
    vbe_draw_circle(cx, cy, 139, 0x2200F2FE);
    vbe_draw_circle(cx, cy, 90, 0x1A00F2FE);

    /* Stylized Geometric Eagle Wings */
    vbe_draw_line(cx - 100, cy - 20, cx, cy + 50, 0x2A00F2FE);
    vbe_draw_line(cx + 100, cy - 20, cx, cy + 50, 0x2A00F2FE);
    vbe_draw_line(cx - 80, cy - 40, cx, cy + 30, 0x2A00F2FE);
    vbe_draw_line(cx + 80, cy - 40, cx, cy + 30, 0x2A00F2FE);

    font_draw_string(cx - 36, cy + 60, "AIRLYS", 0x3300F2FE);
}

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
    vbe_fill_rect(btn_x + 40, win->y + 3, 16, 20, 0x33FF3366);
    font_draw_string(btn_x + 44, win->y + 5, "x", COLOR_CRIMSON);

    /* 4. Window Content Area */
    if (win->draw_content) {
        win->draw_content(win);
    }

    /* 5. Outer 1px Electric Cyan Glow Border */
    uint32_t border_color = (win->id == g_active_window_id) ? COLOR_CYAN_GLOW : COLOR_BORDER_CYAN;
    vbe_draw_glow_rect(win->x, win->y, win->w, win->h, border_color, COLOR_CYAN_DARK);
}

static void draw_taskbar(vbe_screen_t* scr) {
    uint32_t bar_h = 44;
    uint32_t bar_y = scr->height - bar_h - 10;
    uint32_t dock_w = 520;
    uint32_t dock_x = (scr->width / 2) - (dock_w / 2);

    /* Centered Floating Frosted Glass Dock */
    vbe_fill_rect(dock_x, bar_y, dock_w, bar_h, COLOR_TASKBAR_BG);
    vbe_draw_glow_rect(dock_x, bar_y, dock_w, bar_h, COLOR_CYAN_GLOW, COLOR_CYAN_DARK);

    /* Start Button with Eagle Crest */
    uint32_t sx = dock_x + 15;
    uint32_t sy = bar_y + 8;
    vbe_fill_rect(sx, sy, 28, 28, 0xFF003344);
    vbe_draw_rect(sx, sy, 28, 28, COLOR_CYAN_GLOW);
    font_draw_char(sx + 10, sy + 6, 'A', COLOR_CYAN_GLOW);

    /* Pinned Sovereign App Glyphs (No stickers, sharp vector typography) */
    const char* apps[] = {"[Term]", "[Files]", "[Code]", "[Store]", "[Task]", "[Search]", "[Settings]"};
    uint32_t app_x = sx + 40;
    for (int i = 0; i < 7; i++) {
        vbe_fill_rect(app_x, sy, 54, 28, 0xFF0F1829);
        vbe_draw_rect(app_x, sy, 54, 28, (i == 0 || i == 4) ? COLOR_CYAN_GLOW : COLOR_DARK_GREY);
        font_draw_string(app_x + 6, sy + 6, apps[i], (i == 0 || i == 4) ? COLOR_CYAN_GLOW : COLOR_WHITE);
        app_x += 60;
    }

    /* Right System Tray (Clock, Link, Volume, Shield) */
    uint32_t tray_x = scr->width - 200;
    vbe_fill_rect(tray_x, bar_y, 190, bar_h, COLOR_TASKBAR_BG);
    vbe_draw_glow_rect(tray_x, bar_y, 190, bar_h, COLOR_BORDER_CYAN, COLOR_CYAN_DARK);

    rtc_time_t t = rtc_get_time();
    char clk_buf[16];
    clk_buf[0] = '0' + (t.hour / 10);
    clk_buf[1] = '0' + (t.hour % 10);
    clk_buf[2] = ':';
    clk_buf[3] = '0' + (t.minute / 10);
    clk_buf[4] = '0' + (t.minute % 10);
    clk_buf[5] = '\0';

    font_draw_string(tray_x + 15, bar_y + 14, clk_buf, COLOR_CYAN_GLOW);
    font_draw_string(tray_x + 65, bar_y + 14, "[eth0] [80%]", COLOR_LIGHT_GREY);
}

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
    font_draw_string(sm_x + 45, uy + 20, "Root Administrator", COLOR_GREEN_OK);

    /* Power Button Glyph */
    font_draw_string(sm_x + sm_w - 70, uy + 11, "[Power]", COLOR_CRIMSON);
}

void compositor_render_frame(void) {
    vbe_screen_t* scr = vbe_get_screen();
    if (!scr || !scr->initialized) return;

    /* 1. If Lock Screen active, draw lock screen and exit */
    if (g_lock_screen_open) {
        app_lockscreen_draw();
        vbe_swap_buffers();
        return;
    }

    /* 2. Desktop Wallpaper */
    draw_desktop_wallpaper(scr);

    /* 3. Render Windows */
    for (int i = 0; i < MAX_WINDOWS; i++) {
        if (g_windows[i].active && g_windows[i].id != g_active_window_id) {
            draw_window(&g_windows[i]);
        }
    }
    /* Render active window on top */
    for (int i = 0; i < MAX_WINDOWS; i++) {
        if (g_windows[i].active && g_windows[i].id == g_active_window_id) {
            draw_window(&g_windows[i]);
            break;
        }
    }

    /* 4. Taskbar */
    draw_taskbar(scr);

    /* 5. Start Menu Overlay */
    draw_start_menu(scr);

    /* 6. Push to Screen */
    vbe_swap_buffers();
}
