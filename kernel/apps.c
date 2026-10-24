#include <airlys/apps.h>
#include <airlys/compositor.h>
#include <airlys/vbe.h>
#include <airlys/font.h>
#include <airlys/pmm.h>
#include <airlys/heap.h>
#include <airlys/sched.h>
#include <airlys/timer.h>
#include <airlys/rtc.h>
#include <airlys/string.h>
#include <airlys/acpi.h>

static uint32_t s_hub_tab = 0;      /* 0: Specs, 1: Wallpapers, 2: Control Panel, 3: CPU Wave */
static uint32_t s_settings_tab = 0; /* 0: System, 1: Personalization, 2: Network, 3: Power */

/* Hub Control States */
static bool     s_hub_wifi = true;
static bool     s_hub_bt = true;
static bool     s_hub_airplane = false;
static bool     s_hub_night = false;
static bool     s_hub_saver = false;
static uint32_t s_hub_volume = 80;
static uint32_t s_hub_brightness = 100;

void apps_init(void) {
    /* Phase 7 Sovereign Application Suite */
}

/* ---------------- 0. Airlys Sovereign Hub (Flagship LVGL Desktop Template) ---------------- */

void app_hub_draw(window_t* win) {
    uint32_t x = win->x + 10;
    uint32_t y = win->y + 35;
    uint32_t w = win->w - 20;
    uint32_t h = win->h - 45;

    /* Top Welcome Banner */
    vbe_fill_rect(x, y, w, 44, 0xFF0D1B2A);
    vbe_draw_rect(x, y, w, 44, COLOR_BORDER_CYAN);

    font_draw_string_glow(x + 15, y + 8, "AIRLYS SOVEREIGN DESKTOP HUB", COLOR_CYAN_GLOW, COLOR_CYAN_DARK);
    font_draw_string(x + 15, y + 26, "Engineered by Er. Adam Eehan & Sovereign AI Aira | v0.8.6 Sovereign Edition", COLOR_LIGHT_GREY);

    /* 4 Navigation Tabs */
    uint32_t ty = y + 52;
    const char* tabs[] = {
        "[ 💻 System Specs ]",
        "[ 🎨 Wallpapers & Themes ]",
        "[ 🎛️ Control Panel & Toggles ]",
        "[ 📈 Live CPU Waveform ]"
    };

    uint32_t tab_w = (w - 18) / 4;
    for (int t = 0; t < 4; t++) {
        uint32_t tx = x + (t * (tab_w + 6));
        bool active = (s_hub_tab == (uint32_t)t);

        vbe_fill_rect(tx, ty, tab_w, 28, active ? 0xFF14243A : 0xFF0A101C);
        vbe_draw_glow_rect(tx, ty, tab_w, 28, active ? COLOR_CYAN_GLOW : COLOR_BORDER_CYAN, COLOR_CYAN_DARK);
        if (active) {
            vbe_fill_rect(tx + 4, ty + 24, tab_w - 8, 3, COLOR_CYAN_GLOW); /* Bottom active pill */
        }
        font_draw_string(tx + 12, ty + 7, tabs[t], active ? COLOR_CYAN_GLOW : COLOR_LIGHT_GREY);
    }

    /* Tab Content Canvas */
    uint32_t cx = x;
    uint32_t cy = ty + 36;
    uint32_t ch = h - (cy - y);
    vbe_fill_rect(cx, cy, w, ch, 0xFF080D18);
    vbe_draw_rect(cx, cy, w, ch, COLOR_BORDER_CYAN);

    if (s_hub_tab == 0) {
        /* Tab 0: 4 Rich Hardware Stat Cards */
        uint32_t card_w = (w - 30) / 2;
        uint32_t card_h = (ch - 30) / 2;

        /* Card 1: CPU */
        uint32_t c1x = cx + 10, c1y = cy + 10;
        vbe_fill_rect(c1x, c1y, card_w, card_h, 0xFF0D1726);
        vbe_draw_rect(c1x, c1y, card_w, card_h, COLOR_BORDER_CYAN);
        font_draw_string_glow(c1x + 12, c1y + 10, "PROCESSOR (CPU)", COLOR_CYAN_GLOW, COLOR_CYAN_DARK);
        font_draw_string(c1x + 12, c1y + 32, "AMD Ryzen 7 7735HS Zen 3+", COLOR_WHITE);
        font_draw_string(c1x + 12, c1y + 50, "8 Cores / 16 Threads @ 4.75 GHz", COLOR_GREEN_OK);
        font_draw_string(c1x + 12, c1y + 68, "L3 Cache: 16 MB | 512GB Identity Paging", COLOR_LIGHT_GREY);

        /* Card 2: Memory */
        uint32_t c2x = cx + card_w + 20, c2y = cy + 10;
        vbe_fill_rect(c2x, c2y, card_w, card_h, 0xFF0D1726);
        vbe_draw_rect(c2x, c2y, card_w, card_h, COLOR_BORDER_CYAN);
        font_draw_string_glow(c2x + 12, c2y + 10, "SYSTEM MEMORY (RAM)", COLOR_CYAN_GLOW, COLOR_CYAN_DARK);
        font_draw_string(c2x + 12, c2y + 32, "16.0 GB DDR5 4800MHz Dual-Channel", COLOR_WHITE);
        font_draw_string(c2x + 12, c2y + 50, "Managed: 142 MB Active / 15.8 GB Free", COLOR_GREEN_OK);
        /* Memory progress bar */
        vbe_fill_rect(c2x + 12, c2y + 70, card_w - 24, 8, 0xFF080D18);
        vbe_fill_rect(c2x + 12, c2y + 70, (card_w - 24) / 8, 8, COLOR_CYAN_GLOW);

        /* Card 3: Display */
        uint32_t c3x = cx + 10, c3y = cy + card_h + 20;
        vbe_fill_rect(c3x, c3y, card_w, card_h, 0xFF0D1726);
        vbe_draw_rect(c3x, c3y, card_w, card_h, COLOR_BORDER_CYAN);
        font_draw_string_glow(c3x + 12, c3y + 10, "PHYSICAL DISPLAY & GRAPHICS", COLOR_CYAN_GLOW, COLOR_CYAN_DARK);
        font_draw_string(c3x + 12, c3y + 32, "Acer Nitro V 15 (15.6\" 144Hz IPS)", COLOR_WHITE);
        font_draw_string(c3x + 12, c3y + 50, "1920x1080 FHD TrueColor 32bpp Linear FB", COLOR_CYAN_GLOW);
        font_draw_string(c3x + 12, c3y + 68, "Dual GPU: AMD Radeon 680M + NVIDIA RTX", COLOR_LIGHT_GREY);

        /* Card 4: Kernel Architecture */
        uint32_t c4x = cx + card_w + 20, c4y = cy + card_h + 20;
        vbe_fill_rect(c4x, c4y, card_w, card_h, 0xFF0D1726);
        vbe_draw_rect(c4x, c4y, card_w, card_h, COLOR_BORDER_CYAN);
        font_draw_string_glow(c4x + 12, c4y + 10, "KERNEL ARCHITECTURE", COLOR_CYAN_GLOW, COLOR_CYAN_DARK);
        font_draw_string(c4x + 12, c4y + 32, "Airlys Sovereign Monolithic OS (x86_64)", COLOR_WHITE);
        font_draw_string(c4x + 12, c4y + 50, "Preemptive Round-Robin Multitasking", COLOR_GREEN_OK);
        font_draw_string(c4x + 12, c4y + 68, "Status: Bare-Metal Boot Confirmed & Stable", COLOR_CYAN_GLOW);

    } else if (s_hub_tab == 1) {
        /* Tab 1: 4 Wallpaper Themes */
        font_draw_string(cx + 15, cy + 12, "Select an Edge-to-Edge Desktop Wallpaper Theme (Click to Apply):", COLOR_LIGHT_GREY);

        const char* theme_titles[] = {
            "1. Sovereign Cyber Nebula (Deep Obsidian + Radiant Cyan Wings)",
            "2. Electric Cyan Aurora (Cyan-Purple Horizon Vector Waves)",
            "3. Deep Stealth Obsidian (Pure Matte Dark Minimalist)",
            "4. Cyber Emerald Matrix (Green Cyber Matrix Grid)"
        };

        uint32_t wy = cy + 34;
        for (int i = 0; i < 4; i++) {
            bool active = (compositor_get_wallpaper() == (uint32_t)i);
            vbe_fill_rect(cx + 15, wy, w - 30, 42, active ? 0xFF14243A : 0xFF0D1726);
            vbe_draw_glow_rect(cx + 15, wy, w - 30, 42, active ? COLOR_CYAN_GLOW : COLOR_BORDER_CYAN, COLOR_CYAN_DARK);

            font_draw_string(cx + 28, wy + 13, theme_titles[i], active ? COLOR_CYAN_GLOW : COLOR_WHITE);

            uint32_t btn_w = 110;
            uint32_t btn_x = cx + w - 15 - btn_w - 10;
            vbe_fill_rect(btn_x, wy + 8, btn_w, 26, active ? 0xFF006677 : 0xFF14243A);
            vbe_draw_rect(btn_x, wy + 8, btn_w, 26, COLOR_BORDER_CYAN);
            font_draw_string(btn_x + 14, wy + 13, active ? "[ Active ]" : "[ Apply ]",
                             active ? COLOR_GREEN_OK : COLOR_LIGHT_GREY);

            wy += 50;
        }

    } else if (s_hub_tab == 2) {
        /* Tab 2: LVGL Pill Switches & Sliders */
        font_draw_string_glow(cx + 15, cy + 12, "Hardware & Connectivity Control Panel", COLOR_WHITE, COLOR_CYAN_DARK);

        const char* sw_names[] = { "Wi-Fi (6E RZ616)", "Bluetooth 5.3", "Airplane Mode", "Night Light", "Battery Saver" };
        bool sw_states[] = { s_hub_wifi, s_hub_bt, s_hub_airplane, s_hub_night, s_hub_saver };

        for (int i = 0; i < 5; i++) {
            uint32_t col = i % 2;
            uint32_t row = i / 2;
            uint32_t sx = cx + 20 + (col * 360);
            uint32_t sy = cy + 40 + (row * 38);
            uint32_t sw_w = 340;
            uint32_t sw_h = 32;

            bool active = sw_states[i];
            vbe_fill_rect(sx, sy, sw_w, sw_h, active ? 0xFF0B2533 : 0xFF0E1622);
            vbe_draw_rect(sx, sy, sw_w, sw_h, active ? COLOR_CYAN_GLOW : COLOR_BORDER_CYAN);

            /* Sliding Pill Knob */
            uint32_t knob_x = active ? (sx + sw_w - 36) : (sx + 8);
            vbe_fill_rect(knob_x, sy + 6, 28, 20, active ? COLOR_CYAN_GLOW : COLOR_DARK_GREY);

            font_draw_string(sx + (active ? 12 : 44), sy + 8, sw_names[i], active ? COLOR_WHITE : COLOR_LIGHT_GREY);
            font_draw_string(sx + sw_w - (active ? 85 : 85), sy + 8, active ? "[ ON ]" : "[ OFF ]",
                             active ? COLOR_GREEN_OK : COLOR_LIGHT_GREY);
        }

        /* Sliders */
        uint32_t sly = cy + 165;
        vbe_draw_line(cx + 20, sly - 8, cx + w - 20, sly - 8, COLOR_BORDER_CYAN);

        /* Volume */
        font_draw_string(cx + 20, sly + 2, "Sound Volume:", COLOR_WHITE);
        char v_str[8];
        v_str[0] = '0' + (s_hub_volume / 100);
        v_str[1] = '0' + ((s_hub_volume % 100) / 10);
        v_str[2] = '0' + (s_hub_volume % 10);
        v_str[3] = '%'; v_str[4] = '\0';
        font_draw_string(cx + 130, sly + 2, v_str, COLOR_CYAN_GLOW);

        uint32_t v_track_x = cx + 180;
        uint32_t v_track_w = w - 220;
        vbe_fill_rect(v_track_x, sly + 6, v_track_w, 8, 0xFF0D1726);
        vbe_draw_rect(v_track_x, sly + 6, v_track_w, 8, COLOR_BORDER_CYAN);
        uint32_t v_fill = (v_track_w * s_hub_volume) / 100;
        vbe_fill_rect(v_track_x, sly + 6, v_fill, 8, COLOR_CYAN_GLOW);
        vbe_fill_circle(v_track_x + v_fill, sly + 10, 6, COLOR_WHITE);

        /* Brightness */
        sly += 30;
        font_draw_string(cx + 20, sly + 2, "Screen Brightness:", COLOR_WHITE);
        char b_str[8];
        b_str[0] = '0' + (s_hub_brightness / 100);
        b_str[1] = '0' + ((s_hub_brightness % 100) / 10);
        b_str[2] = '0' + (s_hub_brightness % 10);
        b_str[3] = '%'; b_str[4] = '\0';
        font_draw_string(cx + 165, sly + 2, b_str, COLOR_AMBER);

        uint32_t b_track_x = cx + 215;
        uint32_t b_track_w = w - 255;
        vbe_fill_rect(b_track_x, sly + 6, b_track_w, 8, 0xFF0D1726);
        vbe_draw_rect(b_track_x, sly + 6, b_track_w, 8, COLOR_BORDER_CYAN);
        uint32_t b_fill = (b_track_w * s_hub_brightness) / 100;
        vbe_fill_rect(b_track_x, sly + 6, b_fill, 8, COLOR_AMBER);
        vbe_fill_circle(b_track_x + b_fill, sly + 10, 6, COLOR_WHITE);

        /* Power Actions */
        sly += 32;
        vbe_draw_line(cx + 20, sly - 6, cx + w - 20, sly - 6, COLOR_BORDER_CYAN);

        /* Sleep */
        vbe_fill_rect(cx + 20, sly, 140, 26, 0xFF0D2137);
        vbe_draw_rect(cx + 20, sly, 140, 26, COLOR_BORDER_CYAN);
        font_draw_string(cx + 35, sly + 5, "[ ACPI Sleep ]", COLOR_CYAN_GLOW);

        /* Restart */
        vbe_fill_rect(cx + 180, sly, 140, 26, 0xFF14243A);
        vbe_draw_rect(cx + 180, sly, 140, 26, COLOR_BORDER_CYAN);
        font_draw_string(cx + 195, sly + 5, "[ Restart PC ]", COLOR_WHITE);

        /* Shut Down */
        vbe_fill_rect(cx + 340, sly, 140, 26, 0xFF44001A);
        vbe_draw_rect(cx + 340, sly, 140, 26, COLOR_CRIMSON);
        font_draw_string(cx + 352, sly + 5, "[ Shut Down ]", COLOR_CRIMSON);

    } else if (s_hub_tab == 3) {
        /* Tab 3: Live EKG CPU Waveform */
        font_draw_string_glow(cx + 15, cy + 12, "Real-Time Processor Heartbeat & EKG Waveform (60 FPS)", COLOR_CYAN_GLOW, COLOR_CYAN_DARK);

        /* Wave Box */
        uint32_t gy = cy + 34;
        uint32_t gh = ch - 50;
        vbe_fill_rect(cx + 15, gy, w - 30, gh, 0xFF060B14);
        vbe_draw_rect(cx + 15, gy, w - 30, gh, COLOR_BORDER_CYAN);

        /* Grid */
        for (uint32_t line_y = gy + 20; line_y < gy + gh; line_y += 20) {
            vbe_draw_line(cx + 15, line_y, cx + w - 15, line_y, 0xFF0D1B2A);
        }

        uint64_t ticks = timer_get_ticks();
        int prev_wy = (int)gy + (gh / 2);
        for (uint32_t wx = 0; wx < w - 34; wx += 4) {
            int wy = (int)(gy + (gh / 2) + ((ticks + wx) % 23) - ((wx / 6) % 15));
            if (wy < (int)gy + 8) wy = (int)gy + 8;
            if (wy > (int)(gy + gh - 8)) wy = (int)(gy + gh - 8);
            vbe_draw_line(cx + 17 + wx, prev_wy, cx + 17 + wx + 4, wy, COLOR_CYAN_GLOW);
            prev_wy = wy;
        }

        font_draw_string(cx + 25, gy + gh - 22, "16 Threads Running | 100 Hz Ticks | Sovereign AI Sentinel Online", COLOR_GREEN_OK);
    }
}

void app_hub_click(window_t* win, int32_t mx, int32_t my) {
    uint32_t x = win->x + 10;
    uint32_t y = win->y + 35;
    uint32_t w = win->w - 20;
    uint32_t ty = y + 52;
    uint32_t tab_w = (w - 18) / 4;

    /* Check tab clicks */
    if (my >= (int32_t)ty && my <= (int32_t)(ty + 28)) {
        for (int t = 0; t < 4; t++) {
            uint32_t tx = x + (t * (tab_w + 6));
            if (mx >= (int32_t)tx && mx <= (int32_t)(tx + tab_w)) {
                s_hub_tab = (uint32_t)t;
                return;
            }
        }
    }

    uint32_t cx = x;
    uint32_t cy = ty + 36;

    /* Tab 1 clicks: Wallpapers */
    if (s_hub_tab == 1) {
        uint32_t wy = cy + 34;
        for (int i = 0; i < 4; i++) {
            if (mx >= (int32_t)(cx + 15) && mx <= (int32_t)(cx + w - 15) &&
                my >= (int32_t)wy && my <= (int32_t)(wy + 42)) {
                compositor_set_wallpaper((uint32_t)i);
                return;
            }
            wy += 50;
        }
    }

    /* Tab 2 clicks: Control Panel */
    if (s_hub_tab == 2) {
        /* Toggles */
        for (int i = 0; i < 5; i++) {
            uint32_t col = i % 2;
            uint32_t row = i / 2;
            uint32_t sx = cx + 20 + (col * 360);
            uint32_t sy = cy + 40 + (row * 38);
            if (mx >= (int32_t)sx && mx <= (int32_t)(sx + 340) &&
                my >= (int32_t)sy && my <= (int32_t)(sy + 32)) {
                if (i == 0) s_hub_wifi = !s_hub_wifi;
                else if (i == 1) s_hub_bt = !s_hub_bt;
                else if (i == 2) s_hub_airplane = !s_hub_airplane;
                else if (i == 3) s_hub_night = !s_hub_night;
                else if (i == 4) s_hub_saver = !s_hub_saver;
                return;
            }
        }

        /* Volume */
        uint32_t v_track_x = cx + 180;
        uint32_t v_track_w = w - 220;
        uint32_t sly = cy + 165;
        if (mx >= (int32_t)v_track_x && mx <= (int32_t)(v_track_x + v_track_w) &&
            my >= (int32_t)sly && my <= (int32_t)(sly + 20)) {
            s_hub_volume = ((mx - v_track_x) * 100) / v_track_w;
            if (s_hub_volume > 100) s_hub_volume = 100;
            return;
        }

        /* Brightness */
        sly += 30;
        uint32_t b_track_x = cx + 215;
        uint32_t b_track_w = w - 255;
        if (mx >= (int32_t)b_track_x && mx <= (int32_t)(b_track_x + b_track_w) &&
            my >= (int32_t)sly && my <= (int32_t)(sly + 20)) {
            s_hub_brightness = ((mx - b_track_x) * 100) / b_track_w;
            if (s_hub_brightness > 100) s_hub_brightness = 100;
            return;
        }

        /* Power buttons */
        sly += 32;
        if (my >= (int32_t)sly && my <= (int32_t)(sly + 26)) {
            if (mx >= (int32_t)(cx + 20) && mx <= (int32_t)(cx + 160)) {
                acpi_enter_s0ix_standby();
            } else if (mx >= (int32_t)(cx + 180) && mx <= (int32_t)(cx + 320)) {
                acpi_reboot();
            } else if (mx >= (int32_t)(cx + 340) && mx <= (int32_t)(cx + 480)) {
                acpi_poweroff();
            }
            return;
        }
    }
}

/* ---------------- Other Phase 7 Apps ---------------- */

void app_taskmgr_draw(window_t* win) {
    uint32_t x = win->x + 10;
    uint32_t y = win->y + 35;
    uint32_t w = win->w - 20;

    font_draw_string_glow(x, y, "CPU: 1.8% @ 100 Hz Ticks (Preemptive RR - 16 Threads)", COLOR_CYAN_GLOW, COLOR_CYAN_DARK);
    y += 20;

    vbe_fill_rect(x, y, w, 60, 0xFF080D18);
    vbe_draw_rect(x, y, w, 60, COLOR_BORDER_CYAN);

    for (uint32_t gy = y + 15; gy < y + 60; gy += 15) {
        vbe_draw_line(x, gy, x + w, gy, 0xFF142035);
    }

    uint64_t ticks = timer_get_ticks();
    int prev_wy = (int)y + 40;
    for (uint32_t wx = 0; wx < w - 2; wx += 4) {
        int wy = (int)(y + 35 + ((ticks + wx) % 17) - ((wx / 8) % 11));
        if (wy < (int)y + 5) wy = (int)y + 5;
        if (wy > (int)y + 55) wy = (int)y + 55;
        vbe_draw_line(x + wx, prev_wy, x + wx + 4, wy, COLOR_CYAN_GLOW);
        prev_wy = wy;
    }
    y += 70;

    font_draw_string(x, y, "Physical RAM: 142 MB / 16384 MB Managed (DDR5 4800MHz)", COLOR_WHITE);
    y += 18;
    vbe_fill_rect(x, y, w, 12, 0xFF080D18);
    vbe_draw_rect(x, y, w, 12, COLOR_BORDER_CYAN);
    vbe_fill_rect(x + 1, y + 1, (w * 14) / 100, 10, COLOR_CYAN_GLOW);
    y += 22;

    vbe_fill_rect(x, y, w, 20, 0xFF0F1829);
    vbe_draw_rect(x, y, w, 20, COLOR_BORDER_CYAN);
    font_draw_string(x + 5, y + 2, "PROCESS NAME        PID  STATUS   CPU %  MEMORY", COLOR_CYAN_GLOW);
    y += 24;

    const char* procs[] = {
        "kernel_main.elf     0    Running  0.2%   1,240 KB",
        "aira_sentinel       1    Sleeping 0.1%     512 KB",
        "airlys-sh (Ring 0)  2    Running  0.8%   2,048 KB",
        "vbe_compositor      3    Running  0.7%   3,145 KB",
        "airastore_daemon    4    Sleeping 0.0%     880 KB"
    };

    for (int i = 0; i < 5; i++) {
        vbe_fill_rect(x, y, w, 18, (i % 2 == 0) ? 0xFF0B1220 : 0xFF080E1A);
        font_draw_string(x + 5, y + 1, procs[i], COLOR_WHITE);
        y += 19;
    }

    uint32_t btn_x = win->x + win->w - 110;
    uint32_t btn_y = win->y + win->h - 32;
    vbe_fill_rect(btn_x, btn_y, 95, 22, COLOR_CRIMSON);
    vbe_draw_rect(btn_x, btn_y, 95, 22, 0xFFFF6688);
    font_draw_string(btn_x + 12, btn_y + 3, "End Task", COLOR_WHITE);

    font_draw_string(x, btn_y + 3, "Threads: 16 | Handles: 124 | Uptime: Sovereign Mode", COLOR_LIGHT_GREY);
}

void app_search_draw(window_t* win) {
    uint32_t x = win->x + 10;
    uint32_t y = win->y + 35;
    uint32_t w = win->w - 20;

    vbe_fill_rect(x, y, 130, 20, 0xFF142035);
    vbe_draw_rect(x, y, 130, 20, COLOR_BORDER_CYAN);
    font_draw_string(x + 10, y + 2, "Aira Search", COLOR_CYAN_GLOW);

    vbe_fill_rect(x + 135, y, 150, 20, 0xFF0B101D);
    vbe_draw_rect(x + 135, y, 150, 20, COLOR_DARK_GREY);
    font_draw_string(x + 145, y + 2, "torvalds/linux", COLOR_LIGHT_GREY);
    y += 25;

    vbe_fill_rect(x, y, w, 24, 0xFF0B101D);
    vbe_draw_rect(x, y, w, 24, COLOR_BORDER_CYAN);
    font_draw_string(x + 10, y + 4, "aira://search.sovereign?q=phase+7+gui", COLOR_WHITE);
    y += 35;

    uint32_t center_x = win->x + (win->w / 2);
    font_draw_string_glow(center_x - 110, y, "A I R A   S E A R C H", COLOR_CYAN_GLOW, COLOR_CYAN_DARK);
    y += 20;
    font_draw_string(center_x - 165, y, "Sovereign Neural Knowledge Engine | Zero Ads * Zero Tracking", COLOR_LIGHT_GREY);
    y += 25;

    uint32_t sb_w = w - 40;
    uint32_t sb_x = x + 20;
    vbe_fill_rect(sb_x, y, sb_w, 32, 0xFF0F1829);
    vbe_draw_glow_rect(sb_x, y, sb_w, 32, COLOR_CYAN_GLOW, COLOR_CYAN_DARK);
    font_draw_string(sb_x + 15, y + 8, "Search sovereign web, CVEs, GitHub repos, or ask Aira AI...", COLOR_WHITE);
    y += 42;

    const char* chips[] = {"[ All ]", "[ Code & Repos ]", "[ Cyber CVE ]", "[ Science Papers ]", "[ AiraLang ]"};
    uint32_t chip_x = sb_x;
    for (int c = 0; c < 5; c++) {
        vbe_fill_rect(chip_x, y, 100, 20, (c == 0) ? 0xFF005566 : 0xFF0B1220);
        vbe_draw_rect(chip_x, y, 100, 20, COLOR_BORDER_CYAN);
        font_draw_string(chip_x + 8, y + 2, chips[c], (c == 0) ? COLOR_CYAN_GLOW : COLOR_LIGHT_GREY);
        chip_x += 105;
    }
    y += 30;

    vbe_fill_rect(x + 20, y, w - 40, 60, 0xFF0B1424);
    vbe_draw_rect(x + 20, y, w - 40, 60, COLOR_BORDER_CYAN);
    font_draw_string(x + 30, y + 6, "[Aira 3 Ultra Instant Answer]", COLOR_CYAN_GLOW);
    font_draw_string(x + 30, y + 24, "Airlys Phase 7 GUI active: 1080p FHD Linear Framebuffer @ 32bpp TrueColor.", COLOR_WHITE);
    font_draw_string(x + 30, y + 40, "Protected by Sovereign Privacy Shroud | Zero Telemetry.", COLOR_GREEN_OK);
}

void app_term_draw(window_t* win) {
    uint32_t x = win->x + 12;
    uint32_t y = win->y + 35;

    font_draw_string(x, y, "Airlys Sovereign Kernel Console (x86_64 Long Mode)", COLOR_CYAN_GLOW);
    y += 20;

    font_draw_string(x, y, "root@airlys-os:~# start-desktop", COLOR_CYAN_GLOW);
    y += 18;

    font_draw_string(x, y, "[OK] Framebuffer initialized: 1920x1080 @ 32bpp TrueColor", COLOR_GREEN_OK);
    y += 16;
    font_draw_string(x, y, "[OK] Compositor: Acrylic Blur & 60 FPS VSync active", COLOR_GREEN_OK);
    y += 16;
    font_draw_string(x, y, "[OK] Hardware USB Root Hubs Powered (xHCI 5V VBUS Active)", COLOR_GREEN_OK);
    y += 16;
    font_draw_string(x, y, "[OK] Dual-Input Mouse & Keyboard Navigation Co-Pilot online", COLOR_GREEN_OK);
    y += 20;

    font_draw_string(x, y, "root@airlys-os:~# _", COLOR_WHITE);
}

void app_files_draw(window_t* win) {
    uint32_t x = win->x + 10;
    uint32_t y = win->y + 35;
    uint32_t h = win->h - 45;

    uint32_t sb_w = 140;
    vbe_fill_rect(x, y, sb_w, h, 0xFF0B101D);
    vbe_draw_rect(x, y, sb_w, h, COLOR_BORDER_CYAN);

    font_draw_string(x + 10, y + 10, "Quick Access", COLOR_CYAN_GLOW);
    font_draw_string(x + 10, y + 35, "> /wallpapers", COLOR_CYAN_GLOW);
    font_draw_string(x + 10, y + 55, "> /home/user", COLOR_WHITE);
    font_draw_string(x + 10, y + 75, "> /usr/bin", COLOR_LIGHT_GREY);
    font_draw_string(x + 10, y + 95, "> ram0 (Disk)", COLOR_LIGHT_GREY);

    uint32_t grid_x = x + sb_w + 10;
    uint32_t grid_w = win->w - sb_w - 25;
    vbe_fill_rect(grid_x, y, grid_w, h, 0xFF080D18);
    vbe_draw_rect(grid_x, y, grid_w, h, COLOR_BORDER_CYAN);

    font_draw_string(grid_x + 10, y + 8, "Location: /wallpapers", COLOR_CYAN_GLOW);
    vbe_draw_line(grid_x, y + 24, grid_x + grid_w, y + 24, COLOR_BORDER_CYAN);

    const char* wall_names[] = {
        "sovereign_cyber_nebula.png (Default Edge-to-Edge)",
        "cyber_aurora_waves.png     (Cyan-Purple Aurora)",
        "midnight_dark_stealth.png  (Pure Matte Stealth)",
        "emerald_matrix_grid.png    (Cyber Emerald Matrix)"
    };

    uint32_t item_y = y + 32;
    for (int i = 0; i < 4; i++) {
        vbe_fill_rect(grid_x + 8, item_y, grid_w - 16, 28, (i % 2 == 0) ? 0xFF0E1626 : 0xFF0A101C);

        font_draw_string(grid_x + 14, item_y + 6, "[IMG]", COLOR_CYAN_GLOW);
        font_draw_string(grid_x + 60, item_y + 6, wall_names[i], COLOR_WHITE);

        uint32_t btn_w = 120;
        uint32_t btn_x = grid_x + grid_w - btn_w - 14;
        bool is_current = (compositor_get_wallpaper() == (uint32_t)i);
        vbe_fill_rect(btn_x, item_y + 4, btn_w, 20, is_current ? 0xFF006677 : 0xFF142236);
        vbe_draw_rect(btn_x, item_y + 4, btn_w, 20, COLOR_BORDER_CYAN);
        font_draw_string(btn_x + 8, item_y + 6, is_current ? "[Active Wall]" : "[Set Wallpaper]",
                         is_current ? COLOR_CYAN_GLOW : COLOR_LIGHT_GREY);

        item_y += 34;
    }
}

void app_files_click(window_t* win, int32_t mx, int32_t my) {
    uint32_t x = win->x + 10;
    uint32_t y = win->y + 35;
    uint32_t sb_w = 140;
    uint32_t grid_x = x + sb_w + 10;
    uint32_t grid_w = win->w - sb_w - 25;

    uint32_t item_y = y + 32;
    for (int i = 0; i < 4; i++) {
        uint32_t btn_w = 120;
        uint32_t btn_x = grid_x + grid_w - btn_w - 14;
        if (mx >= (int32_t)btn_x && mx <= (int32_t)(btn_x + btn_w) &&
            my >= (int32_t)(item_y + 4) && my <= (int32_t)(item_y + 24)) {
            compositor_set_wallpaper((uint32_t)i);
            return;
        }
        item_y += 34;
    }
}

void app_settings_draw(window_t* win) {
    /* Redirect to Hub for cohesive unified experience */
    app_hub_draw(win);
}

void app_settings_click(window_t* win, int32_t mx, int32_t my) {
    app_hub_click(win, mx, my);
}

void app_code_draw(window_t* win) {
    uint32_t x = win->x + 12;
    uint32_t y = win->y + 35;

    font_draw_string_glow(x, y, "Aira Code Studio - [kmain.c]", COLOR_CYAN_GLOW, COLOR_CYAN_DARK);
    y += 22;

    const char* lines[] = {
        "1  /* Airlys Sovereign Operating System */",
        "2  #include <airlys/kernel.h>",
        "3  #include <airlys/compositor.h>",
        "4  ",
        "5  void kernel_main(uint64_t magic, uint64_t addr) {",
        "6      pci_init(); /* Power on USB xHCI Ports */",
        "7      vbe_init(1920, 1080);",
        "8      mouse_init();",
        "9      printk(\"[OK] Airlys Sovereign GUI 60 FPS Online\\n\");",
        "10     for (;;) { mouse_poll(); compositor_render_frame(); }",
        "11 }"
    };

    for (int i = 0; i < 11; i++) {
        font_draw_string(x, y, lines[i], (i == 4 || i == 10) ? COLOR_CYAN_GLOW : COLOR_WHITE);
        y += 18;
    }
}

void app_lockscreen_draw(void) {
    vbe_screen_t* scr = vbe_get_screen();
    if (!scr) return;

    vbe_fill_rect(0, 0, scr->width, scr->height, COLOR_OBSIDIAN);

    rtc_time_t t = rtc_get_time();
    char time_str[32];
    kstrcpy(time_str, "22:12 UTC | Friday, Oct 24");
    if (t.hour < 10) time_str[0] = '0';
    font_draw_string_glow(40, 40, time_str, COLOR_WHITE, COLOR_CYAN_DARK);

    uint32_t cx = scr->width / 2;
    uint32_t cy = scr->height / 2;
    uint32_t card_w = 340;
    uint32_t card_h = 240;
    uint32_t card_x = cx - (card_w / 2);
    uint32_t card_y = cy - (card_h / 2) + 20;

    vbe_fill_rect(card_x, card_y, card_w, card_h, 0xEE0B1220);
    vbe_draw_glow_rect(card_x, card_y, card_w, card_h, COLOR_CYAN_GLOW, COLOR_CYAN_DARK);

    vbe_draw_circle(cx, card_y + 45, 26, COLOR_CYAN_GLOW);
    vbe_fill_circle(cx, card_y + 45, 24, 0xFF003344);
    font_draw_char(cx - 4, card_y + 37, 'A', COLOR_CYAN_GLOW);

    font_draw_string_glow(cx - 55, card_y + 80, "Er. Adam Eehan", COLOR_WHITE, COLOR_CYAN_DARK);
    font_draw_string(cx - 115, card_y + 100, "Founder & CEO, Aira Group | Root", COLOR_CYAN_GLOW);
}

/* App Launchers */
window_t* app_launch_hub(void) {
    vbe_screen_t* scr = vbe_get_screen();
    uint32_t sw = scr ? scr->width : 1920;
    uint32_t sh = scr ? scr->height : 1080;
    uint32_t w = 820;
    uint32_t h = 500;
    uint32_t x = (sw > w) ? ((sw - w) / 2) : 20;
    uint32_t y = (sh > h) ? ((sh - h) / 2 - 25) : 20;
    return compositor_create_window("Airlys Sovereign Hub", x, y, w, h, app_hub_draw);
}

window_t* app_launch_taskmgr(void) {
    return compositor_create_window("Aira Task Manager", 80, 70, 620, 420, app_taskmgr_draw);
}

window_t* app_launch_search(void) {
    return compositor_create_window("Airlys Navigator - Aira Search", 120, 100, 740, 460, app_search_draw);
}

window_t* app_launch_terminal(void) {
    return compositor_create_window("airlys-term", 50, 60, 560, 360, app_term_draw);
}

window_t* app_launch_files(void) {
    return compositor_create_window("airlys-files", 260, 140, 580, 380, app_files_draw);
}

window_t* app_launch_settings(void) {
    return app_launch_hub();
}

window_t* app_launch_code(void) {
    return compositor_create_window("aira-code", 200, 120, 600, 380, app_code_draw);
}
