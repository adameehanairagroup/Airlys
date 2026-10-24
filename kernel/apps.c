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

static uint32_t s_settings_tab = 0; /* 0: System, 1: Personalization, 2: Network, 3: Power */

void apps_init(void) {
    /* Initialize Phase 7 application registry */
}

/* 1. Aira Task Manager Window */
void app_taskmgr_draw(window_t* win) {
    uint32_t x = win->x + 10;
    uint32_t y = win->y + 35;
    uint32_t w = win->w - 20;

    /* Top Stats Bar */
    font_draw_string_glow(x, y, "CPU: 1.8% @ 100 Hz Ticks (Preemptive RR - 16 Threads)", COLOR_CYAN_GLOW, COLOR_CYAN_DARK);
    y += 20;

    /* Live CPU Wave Graph Box */
    vbe_fill_rect(x, y, w, 60, 0xFF080D18);
    vbe_draw_rect(x, y, w, 60, COLOR_BORDER_CYAN);

    /* Grid lines */
    for (uint32_t gy = y + 15; gy < y + 60; gy += 15) {
        vbe_draw_line(x, gy, x + w, gy, 0xFF142035);
    }

    /* Pulsating wave line */
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

    /* Memory Gauge */
    font_draw_string(x, y, "Physical RAM: 142 MB / 16384 MB Managed (DDR5 4800MHz)", COLOR_WHITE);
    y += 18;
    vbe_fill_rect(x, y, w, 12, 0xFF080D18);
    vbe_draw_rect(x, y, w, 12, COLOR_BORDER_CYAN);
    vbe_fill_rect(x + 1, y + 1, (w * 14) / 100, 10, COLOR_CYAN_GLOW);
    y += 22;

    /* Process Table Header */
    vbe_fill_rect(x, y, w, 20, 0xFF0F1829);
    vbe_draw_rect(x, y, w, 20, COLOR_BORDER_CYAN);
    font_draw_string(x + 5, y + 2, "PROCESS NAME        PID  STATUS   CPU %  MEMORY", COLOR_CYAN_GLOW);
    y += 24;

    /* Active Task Rows */
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

    /* Bottom Action: End Task Button */
    uint32_t btn_x = win->x + win->w - 110;
    uint32_t btn_y = win->y + win->h - 32;
    vbe_fill_rect(btn_x, btn_y, 95, 22, COLOR_CRIMSON);
    vbe_draw_rect(btn_x, btn_y, 95, 22, 0xFFFF6688);
    font_draw_string(btn_x + 12, btn_y + 3, "End Task", COLOR_WHITE);

    font_draw_string(x, btn_y + 3, "Threads: 16 | Handles: 124 | Uptime: Sovereign Mode", COLOR_LIGHT_GREY);
}

/* 2. Aira Search & Web Navigator Window */
void app_search_draw(window_t* win) {
    uint32_t x = win->x + 10;
    uint32_t y = win->y + 35;
    uint32_t w = win->w - 20;

    /* Browser Tabs */
    vbe_fill_rect(x, y, 130, 20, 0xFF142035);
    vbe_draw_rect(x, y, 130, 20, COLOR_BORDER_CYAN);
    font_draw_string(x + 10, y + 2, "Aira Search", COLOR_CYAN_GLOW);

    vbe_fill_rect(x + 135, y, 150, 20, 0xFF0B101D);
    vbe_draw_rect(x + 135, y, 150, 20, COLOR_DARK_GREY);
    font_draw_string(x + 145, y + 2, "torvalds/linux", COLOR_LIGHT_GREY);
    y += 25;

    /* URL / Address Bar */
    vbe_fill_rect(x, y, w, 24, 0xFF0B101D);
    vbe_draw_rect(x, y, w, 24, COLOR_BORDER_CYAN);
    font_draw_string(x + 10, y + 4, "aira://search.sovereign?q=phase+7+gui", COLOR_WHITE);
    y += 35;

    /* Center Branding */
    uint32_t center_x = win->x + (win->w / 2);
    font_draw_string_glow(center_x - 110, y, "A I R A   S E A R C H", COLOR_CYAN_GLOW, COLOR_CYAN_DARK);
    y += 20;
    font_draw_string(center_x - 165, y, "Sovereign Neural Knowledge Engine | Zero Ads * Zero Tracking", COLOR_LIGHT_GREY);
    y += 25;

    /* Search Input Pill */
    uint32_t sb_w = w - 40;
    uint32_t sb_x = x + 20;
    vbe_fill_rect(sb_x, y, sb_w, 32, 0xFF0F1829);
    vbe_draw_glow_rect(sb_x, y, sb_w, 32, COLOR_CYAN_GLOW, COLOR_CYAN_DARK);
    font_draw_string(sb_x + 15, y + 8, "Search sovereign web, CVEs, GitHub repos, or ask Aira AI...", COLOR_WHITE);
    y += 42;

    /* Filter Chips */
    const char* chips[] = {"[ All ]", "[ Code & Repos ]", "[ Cyber CVE ]", "[ Science Papers ]", "[ AiraLang ]"};
    uint32_t chip_x = sb_x;
    for (int c = 0; c < 5; c++) {
        vbe_fill_rect(chip_x, y, 100, 20, (c == 0) ? 0xFF005566 : 0xFF0B1220);
        vbe_draw_rect(chip_x, y, 100, 20, COLOR_BORDER_CYAN);
        font_draw_string(chip_x + 8, y + 2, chips[c], (c == 0) ? COLOR_CYAN_GLOW : COLOR_LIGHT_GREY);
        chip_x += 105;
    }
    y += 30;

    /* Direct AI Answer Box */
    vbe_fill_rect(x + 20, y, w - 40, 60, 0xFF0B1424);
    vbe_draw_rect(x + 20, y, w - 40, 60, COLOR_BORDER_CYAN);
    font_draw_string(x + 30, y + 6, "[Aira 3 Ultra Instant Answer]", COLOR_CYAN_GLOW);
    font_draw_string(x + 30, y + 24, "Airlys Phase 7 GUI active: 1080p FHD Linear Framebuffer @ 32bpp TrueColor.", COLOR_WHITE);
    font_draw_string(x + 30, y + 40, "Protected by Sovereign Privacy Shroud | Zero Telemetry.", COLOR_GREEN_OK);
}

/* 3. Translucent Sovereign Terminal Window */
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
    font_draw_string(x, y, "[OK] PS/2 Touchpad & Mouse Driver bound (IRQ 12 + Polling)", COLOR_GREEN_OK);
    y += 16;
    font_draw_string(x, y, "[OK] Dual-Input Keyboard Cursor Navigation co-pilot online", COLOR_GREEN_OK);
    y += 20;

    font_draw_string(x, y, "root@airlys-os:~# _", COLOR_WHITE);
}

/* 4. Sovereign File Explorer Window */
void app_files_draw(window_t* win) {
    uint32_t x = win->x + 10;
    uint32_t y = win->y + 35;
    uint32_t h = win->h - 45;

    /* Left Sidebar: Quick Access */
    uint32_t sb_w = 140;
    vbe_fill_rect(x, y, sb_w, h, 0xFF0B101D);
    vbe_draw_rect(x, y, sb_w, h, COLOR_BORDER_CYAN);

    font_draw_string(x + 10, y + 10, "Quick Access", COLOR_CYAN_GLOW);
    font_draw_string(x + 10, y + 35, "> /wallpapers", COLOR_CYAN_GLOW);
    font_draw_string(x + 10, y + 55, "> /home/user", COLOR_WHITE);
    font_draw_string(x + 10, y + 75, "> /usr/bin", COLOR_LIGHT_GREY);
    font_draw_string(x + 10, y + 95, "> ram0 (Disk)", COLOR_LIGHT_GREY);

    /* Main File Grid */
    uint32_t grid_x = x + sb_w + 10;
    uint32_t grid_w = win->w - sb_w - 25;
    vbe_fill_rect(grid_x, y, grid_w, h, 0xFF080D18);
    vbe_draw_rect(grid_x, y, grid_w, h, COLOR_BORDER_CYAN);

    font_draw_string(grid_x + 10, y + 8, "Location: /wallpapers", COLOR_CYAN_GLOW);
    vbe_draw_line(grid_x, y + 24, grid_x + grid_w, y + 24, COLOR_BORDER_CYAN);

    /* Wallpaper Files with direct "Set as Wallpaper" Action */
    const char* wall_names[] = {
        "sovereign_logo.png   (Default Airlys Logo)",
        "cyber_aurora.png     (Cyan Aurora Theme)",
        "midnight_dark.png    (Pure Matte Stealth)",
        "emerald_matrix.png   (Cyber Matrix Theme)"
    };

    uint32_t item_y = y + 32;
    for (int i = 0; i < 4; i++) {
        vbe_fill_rect(grid_x + 8, item_y, grid_w - 16, 28, (i % 2 == 0) ? 0xFF0E1626 : 0xFF0A101C);

        /* Icon & Name */
        font_draw_string(grid_x + 14, item_y + 6, "[IMG]", COLOR_CYAN_GLOW);
        font_draw_string(grid_x + 60, item_y + 6, wall_names[i], COLOR_WHITE);

        /* Set Wallpaper Button */
        uint32_t btn_w = 120;
        uint32_t btn_x = grid_x + grid_w - btn_w - 14;
        bool is_current = (compositor_get_wallpaper() == (uint32_t)i);
        vbe_fill_rect(btn_x, item_y + 4, btn_w, 20, is_current ? 0xFF006677 : 0xFF142236);
        vbe_draw_rect(btn_x, item_y + 4, btn_w, 20, COLOR_BORDER_CYAN);
        font_draw_string(btn_x + 8, item_y + 6, is_current ? "[Active Wall]" : "[Set Wallpaper]",
                         is_current ? COLOR_CYAN_GLOW : COLOR_LIGHT_GREY);

        item_y += 34;
    }

    /* Regular Files */
    const char* other_files[] = {
        "[DOC] sovereign_covenant.txt (4 KB)",
        "[ELF] kernel_main.elf (814 KB)"
    };
    for (int i = 0; i < 2; i++) {
        vbe_fill_rect(grid_x + 8, item_y, grid_w - 16, 24, 0xFF0A101C);
        font_draw_string(grid_x + 14, item_y + 4, other_files[i], COLOR_LIGHT_GREY);
        item_y += 28;
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

/* 5. Airlys Settings App Window (LVGL Tabview & System Controls) */
void app_settings_draw(window_t* win) {
    uint32_t x = win->x + 10;
    uint32_t y = win->y + 35;
    uint32_t h = win->h - 45;

    /* Left Sidebar: LVGL Tabview Navigation */
    uint32_t tb_w = 150;
    vbe_fill_rect(x, y, tb_w, h, 0xFF0B101D);
    vbe_draw_rect(x, y, tb_w, h, COLOR_BORDER_CYAN);

    font_draw_string_glow(x + 15, y + 10, "Settings", COLOR_CYAN_GLOW, COLOR_CYAN_DARK);
    vbe_draw_line(x, y + 28, x + tb_w, y + 28, COLOR_BORDER_CYAN);

    const char* tabs[] = {
        "[i] System",
        "[*] Personalize",
        "[~] Network",
        "[#] Power & Bat"
    };

    uint32_t tab_y = y + 36;
    for (int t = 0; t < 4; t++) {
        bool active = (s_settings_tab == (uint32_t)t);
        if (active) {
            vbe_fill_rect(x + 4, tab_y, tb_w - 8, 28, 0xFF14243A);
            vbe_draw_glow_rect(x + 4, tab_y, tb_w - 8, 28, COLOR_CYAN_GLOW, COLOR_CYAN_DARK);
            vbe_fill_rect(x + 4, tab_y, 4, 28, COLOR_CYAN_GLOW); /* Active pill indicator */
        }
        font_draw_string(x + 16, tab_y + 7, tabs[t], active ? COLOR_CYAN_GLOW : COLOR_LIGHT_GREY);
        tab_y += 34;
    }

    /* Main Settings Panel */
    uint32_t main_x = x + tb_w + 10;
    uint32_t main_w = win->w - tb_w - 25;
    vbe_fill_rect(main_x, y, main_w, h, 0xFF080D18);
    vbe_draw_rect(main_x, y, main_w, h, COLOR_BORDER_CYAN);

    uint32_t cy = y + 15;

    if (s_settings_tab == 0) {
        /* Tab 0: System Information */
        font_draw_string_glow(main_x + 15, cy, "System Specifications", COLOR_WHITE, COLOR_CYAN_DARK);
        cy += 24;

        vbe_fill_rect(main_x + 15, cy, main_w - 30, 2, COLOR_BORDER_CYAN);
        cy += 12;

        font_draw_string(main_x + 15, cy, "OS Name        : Airlys Sovereign OS (v0.8.6 Sovereign Release)", COLOR_CYAN_GLOW); cy += 20;
        font_draw_string(main_x + 15, cy, "Lead Architect : Er. Adam Eehan (Founder & CEO, Aira Group)", COLOR_WHITE); cy += 20;
        font_draw_string(main_x + 15, cy, "AI Co-Pilot    : Sovereign AI Aira 3 Ultra (Engine v3.6)", COLOR_GREEN_OK); cy += 20;
        font_draw_string(main_x + 15, cy, "Processor      : AMD Ryzen 7 7735HS Zen 3+ (8 Cores / 16 Threads @ 4.75 GHz)", COLOR_WHITE); cy += 20;
        font_draw_string(main_x + 15, cy, "Installed RAM  : 16.0 GB DDR5 4800MHz Dual-Channel", COLOR_WHITE); cy += 20;
        font_draw_string(main_x + 15, cy, "Graphics       : AMD Radeon 680M / 780M + NVIDIA GeForce RTX", COLOR_WHITE); cy += 20;
        font_draw_string(main_x + 15, cy, "Display Mode   : 1920x1080 FHD 144Hz IPS TrueColor 32bpp (Tag 8 GOP)", COLOR_CYAN_GLOW); cy += 20;
        font_draw_string(main_x + 15, cy, "Kernel Mode    : x86_64 Long Mode with 512GB Identity Paging", COLOR_LIGHT_GREY); cy += 20;

    } else if (s_settings_tab == 1) {
        /* Tab 1: Personalization & Wallpaper Picker */
        font_draw_string_glow(main_x + 15, cy, "Personalization & Wallpaper Themes", COLOR_WHITE, COLOR_CYAN_DARK);
        cy += 20;
        font_draw_string(main_x + 15, cy, "Select active desktop wallpaper theme below:", COLOR_LIGHT_GREY);
        cy += 26;

        const char* themes[] = {
            "1. Sovereign Logo Obsidian (Default Airlys Authentic Logo)",
            "2. Cyber Aurora Cyan (Vibrant Cyan-Purple Vector Aurora)",
            "3. Deep Midnight Stealth (Pure Matte Stealth Dark)",
            "4. Cyber Emerald Matrix (Green Cyber Matrix Grid)"
        };

        for (int i = 0; i < 4; i++) {
            bool active = (compositor_get_wallpaper() == (uint32_t)i);
            vbe_fill_rect(main_x + 15, cy, main_w - 30, 36, active ? 0xFF14243A : 0xFF0E1626);
            vbe_draw_glow_rect(main_x + 15, cy, main_w - 30, 36,
                               active ? COLOR_CYAN_GLOW : COLOR_BORDER_CYAN, COLOR_CYAN_DARK);

            font_draw_string(main_x + 25, cy + 10, themes[i], active ? COLOR_CYAN_GLOW : COLOR_WHITE);
            font_draw_string(main_x + main_w - 90, cy + 10, active ? "[ Active ]" : "[ Apply ]",
                             active ? COLOR_GREEN_OK : COLOR_LIGHT_GREY);
            cy += 44;
        }

    } else if (s_settings_tab == 2) {
        /* Tab 2: Network & Wireless */
        font_draw_string_glow(main_x + 15, cy, "Network & Wireless Connections", COLOR_WHITE, COLOR_CYAN_DARK);
        cy += 24;

        font_draw_string(main_x + 15, cy, "Wi-Fi Adapter  : AMD Wi-Fi 6E RZ616 (160MHz) / MT7922", COLOR_WHITE); cy += 20;
        font_draw_string(main_x + 15, cy, "Status         : Connected (WPA3-SAE Enterprise Secure)", COLOR_GREEN_OK); cy += 20;
        font_draw_string(main_x + 15, cy, "SSID           : Airlys-Mesh-5G", COLOR_CYAN_GLOW); cy += 20;
        font_draw_string(main_x + 15, cy, "IPv4 Address   : 192.168.1.137 / 24", COLOR_WHITE); cy += 20;
        font_draw_string(main_x + 15, cy, "Default Gateway: 192.168.1.1", COLOR_WHITE); cy += 20;
        font_draw_string(main_x + 15, cy, "DNS Over HTTPS : 1.1.1.1 (Cloudflare Sovereign Zero-Log)", COLOR_LIGHT_GREY); cy += 20;
        font_draw_string(main_x + 15, cy, "MAC Address    : 00:E0:4C:68:01:AF", COLOR_LIGHT_GREY); cy += 20;

    } else if (s_settings_tab == 3) {
        /* Tab 3: Power & Battery Management */
        font_draw_string_glow(main_x + 15, cy, "Power, Battery & Thermal Engine", COLOR_WHITE, COLOR_CYAN_DARK);
        cy += 24;

        font_draw_string(main_x + 15, cy, "Power Source   : AC Connected (135W Slim-Tip Fast Charging)", COLOR_GREEN_OK); cy += 20;
        font_draw_string(main_x + 15, cy, "Battery Level  : 98% (54 Wh Lithium-Ion, Healthy)", COLOR_CYAN_GLOW); cy += 20;
        font_draw_string(main_x + 15, cy, "Sleep Engine   : ACPI S0ix Modern Standby (2W Idle Consumption)", COLOR_WHITE); cy += 20;
        font_draw_string(main_x + 15, cy, "Power Mode     : [ Performance ]  [ Balanced ]  [ Power Saver ]", COLOR_WHITE); cy += 20;
        font_draw_string(main_x + 15, cy, "Fan Profile    : Nitro Dual-Fan Auto Acoustic Shroud (Cool)", COLOR_LIGHT_GREY); cy += 20;
    }
}

void app_settings_click(window_t* win, int32_t mx, int32_t my) {
    uint32_t x = win->x + 10;
    uint32_t y = win->y + 35;
    uint32_t tb_w = 150;

    /* Check tab clicks */
    if (mx >= (int32_t)x && mx <= (int32_t)(x + tb_w)) {
        uint32_t tab_y = y + 36;
        for (int t = 0; t < 4; t++) {
            if (my >= (int32_t)tab_y && my <= (int32_t)(tab_y + 28)) {
                s_settings_tab = (uint32_t)t;
                return;
            }
            tab_y += 34;
        }
    }

    /* Check wallpaper card clicks in Tab 1 */
    if (s_settings_tab == 1) {
        uint32_t main_x = x + tb_w + 10;
        uint32_t main_w = win->w - tb_w - 25;
        uint32_t cy = y + 15 + 20 + 26;

        for (int i = 0; i < 4; i++) {
            if (mx >= (int32_t)(main_x + 15) && mx <= (int32_t)(main_x + main_w - 15) &&
                my >= (int32_t)cy && my <= (int32_t)(cy + 36)) {
                compositor_set_wallpaper((uint32_t)i);
                return;
            }
            cy += 44;
        }
    }
}

/* 6. Aira Code Editor Window */
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
        "6      serial_init();",
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

/* 7. Sovereign Full-Screen Lock Screen & Greeter */
void app_lockscreen_draw(void) {
    vbe_screen_t* scr = vbe_get_screen();
    if (!scr) return;

    /* Background Obsidian Wash */
    vbe_fill_rect(0, 0, scr->width, scr->height, COLOR_OBSIDIAN);

    /* Clock & Date (Upper Left) */
    rtc_time_t t = rtc_get_time();
    char time_str[32];
    kstrcpy(time_str, "15:10 UTC | Friday, Oct 24");
    if (t.hour < 10) time_str[0] = '0';
    font_draw_string_glow(40, 40, time_str, COLOR_WHITE, COLOR_CYAN_DARK);

    /* Floating Center Login Card */
    uint32_t cx = scr->width / 2;
    uint32_t cy = scr->height / 2;
    uint32_t card_w = 340;
    uint32_t card_h = 240;
    uint32_t card_x = cx - (card_w / 2);
    uint32_t card_y = cy - (card_h / 2) + 20;

    vbe_fill_rect(card_x, card_y, card_w, card_h, 0xEE0B1220);
    vbe_draw_glow_rect(card_x, card_y, card_w, card_h, COLOR_CYAN_GLOW, COLOR_CYAN_DARK);

    /* Avatar Ring */
    vbe_draw_circle(cx, card_y + 45, 26, COLOR_CYAN_GLOW);
    vbe_fill_circle(cx, card_y + 45, 24, 0xFF003344);
    font_draw_char(cx - 4, card_y + 37, 'A', COLOR_CYAN_GLOW);

    /* User Profile */
    font_draw_string_glow(cx - 55, card_y + 80, "Er. Adam Eehan", COLOR_WHITE, COLOR_CYAN_DARK);
    font_draw_string(cx - 115, card_y + 100, "Founder & CEO, Aira Group | Root", COLOR_CYAN_GLOW);

    /* PIN Password Box */
    uint32_t pin_x = card_x + 40;
    uint32_t pin_y = card_y + 130;
    vbe_fill_rect(pin_x, pin_y, 260, 28, 0xFF070B14);
    vbe_draw_rect(pin_x, pin_y, 260, 28, COLOR_BORDER_CYAN);
    font_draw_string(pin_x + 15, pin_y + 6, "* * * * * * * *", COLOR_CYAN_GLOW);
    font_draw_char(pin_x + 235, pin_y + 6, '>', COLOR_CYAN_GLOW);

    /* Alternate Options */
    font_draw_string(cx - 85, card_y + 175, "[ PIN ]  [ Fingerprint ]", COLOR_LIGHT_GREY);
    font_draw_string(cx - 75, card_y + 195, "Sign in with Aira ID", COLOR_CYAN_GLOW);
}

/* App Launchers */
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
    return compositor_create_window("airlys-settings", 180, 110, 660, 420, app_settings_draw);
}

window_t* app_launch_code(void) {
    return compositor_create_window("aira-code", 200, 120, 600, 380, app_code_draw);
}
