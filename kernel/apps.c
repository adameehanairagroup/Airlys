#include <airlys/apps.h>
#include <airlys/vbe.h>
#include <airlys/font.h>
#include <airlys/pmm.h>
#include <airlys/heap.h>
#include <airlys/sched.h>
#include <airlys/timer.h>
#include <airlys/rtc.h>
#include <airlys/string.h>

void apps_init(void) {
    /* Initialize Phase 7 application registry */
}

/* 1. Aira Task Manager Window */
void app_taskmgr_draw(window_t* win) {
    uint32_t x = win->x + 10;
    uint32_t y = win->y + 35;
    uint32_t w = win->w - 20;

    /* Top Stats Bar */
    font_draw_string_glow(x, y, "CPU: 1.8% @ 100 Hz Ticks (Preemptive RR)", COLOR_CYAN_GLOW, COLOR_CYAN_DARK);
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
    font_draw_string(x, y, "Physical RAM: 142 MB / 1024 MB Managed (14% Active)", COLOR_WHITE);
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

    font_draw_string(x, btn_y + 3, "Threads: 5 | Handles: 124 | Uptime: OK", COLOR_LIGHT_GREY);
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
    font_draw_string(center_x - 165, y, "Sovereign Neural Knowledge Engine | Zero Ads • Zero Tracking", COLOR_LIGHT_GREY);
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
    font_draw_string(x + 30, y + 24, "Airlys Phase 7 GUI active: VBE 1024x768 Linear Framebuffer @ 32bpp TrueColor.", COLOR_WHITE);
    font_draw_string(x + 30, y + 40, "Protected by Sovereign Privacy Shroud | 0 Trackers Blocked.", COLOR_GREEN_OK);
}

/* 3. Translucent Sovereign Terminal Window */
void app_term_draw(window_t* win) {
    uint32_t x = win->x + 12;
    uint32_t y = win->y + 35;

    font_draw_string(x, y, "Airlys Sovereign Kernel Console (x86_64 Long Mode)", COLOR_CYAN_GLOW);
    y += 20;

    font_draw_string(x, y, "┌──(root㉿airlys-os)-[/home/user]", COLOR_CYAN_GLOW);
    y += 16;
    font_draw_string(x, y, "└─Airlys >> start-desktop", COLOR_WHITE);
    y += 20;

    font_draw_string(x, y, "[*] VBE Framebuffer initialized: 1024x768 @ 32bpp TrueColor", COLOR_GREEN_OK);
    y += 16;
    font_draw_string(x, y, "[*] Window Compositor: Acrylic Blur & 60 FPS VSync active", COLOR_GREEN_OK);
    y += 16;
    font_draw_string(x, y, "[OK] Desktop Shell (Phase 7 GUI) loaded successfully.", COLOR_CYAN_GLOW);
    y += 20;

    font_draw_string(x, y, "┌──(root㉿airlys-os)-[/home/user]", COLOR_CYAN_GLOW);
    y += 16;
    font_draw_string(x, y, "└─Airlys >> _", COLOR_CYAN_GLOW);
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
    font_draw_string(x + 10, y + 35, "> /home/user", COLOR_WHITE);
    font_draw_string(x + 10, y + 55, "> /etc", COLOR_LIGHT_GREY);
    font_draw_string(x + 10, y + 75, "> /usr/bin", COLOR_LIGHT_GREY);
    font_draw_string(x + 10, y + 95, "> ram0 (Disk)", COLOR_CYAN_GLOW);

    /* Main File Grid */
    uint32_t grid_x = x + sb_w + 10;
    uint32_t grid_w = win->w - sb_w - 30;
    vbe_fill_rect(grid_x, y, grid_w, h, 0xFF080D18);
    vbe_draw_rect(grid_x, y, grid_w, h, COLOR_BORDER_CYAN);

    /* Files */
    const char* items[] = {
        "[PKG] demo.apkg     (42 KB)  - Aira Package",
        "[SRC] system.aira   (12 KB)  - AiraLang Script",
        "[DOC] covenant.txt   (4 KB)  - Sovereign Covenant",
        "[ELF] kernel.elf   (814 KB)  - Monolithic Kernel",
        "[DIR] /apps                  - Installed Apps"
    };

    uint32_t item_y = y + 15;
    for (int i = 0; i < 5; i++) {
        vbe_fill_rect(grid_x + 10, item_y, grid_w - 20, 24, (i % 2 == 0) ? 0xFF0E1626 : 0xFF0A101C);
        font_draw_string(grid_x + 18, item_y + 4, items[i], (i == 0) ? COLOR_CYAN_GLOW : COLOR_WHITE);
        item_y += 30;
    }
}

/* 5. Sovereign Full-Screen Lock Screen & Greeter */
void app_lockscreen_draw(void) {
    vbe_screen_t* scr = vbe_get_screen();
    if (!scr) return;

    /* Background Obsidian Wash */
    vbe_fill_rect(0, 0, scr->width, scr->height, COLOR_OBSIDIAN);

    /* Glowing Center Watermark */
    uint32_t cx = scr->width / 2;
    uint32_t cy = scr->height / 2;
    vbe_draw_circle(cx, cy - 30, 90, 0x3300F2FE);
    vbe_draw_circle(cx, cy - 30, 89, 0x5500F2FE);

    /* Clock & Date (Upper Left) */
    rtc_time_t t = rtc_get_time();
    char time_str[32];
    kstrcpy(time_str, "15:10 UTC | Friday, Oct 23");
    if (t.hour < 10) time_str[0] = '0';
    font_draw_string_glow(40, 40, time_str, COLOR_WHITE, COLOR_CYAN_DARK);

    /* Floating Center Login Card */
    uint32_t card_w = 340;
    uint32_t card_h = 240;
    uint32_t card_x = cx - (card_w / 2);
    uint32_t card_y = cy - (card_h / 2) + 20;

    vbe_fill_rect(card_x, card_y, card_w, card_h, 0xEE0B1220);
    vbe_draw_glow_rect(card_x, card_y, card_w, card_h, COLOR_CYAN_GLOW, COLOR_CYAN_DARK);

    /* Avatar Cyber Ring */
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

    /* Bottom Status Glyphs */
    font_draw_string(scr->width - 160, scr->height - 30, "[Gigabit] [Vol] [Power]", COLOR_LIGHT_GREY);
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
