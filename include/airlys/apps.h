#ifndef AIRLYS_APPS_H
#define AIRLYS_APPS_H

/*
 * Airlys Operating System Kernel - Phase 7 Sovereign Applications
 * Lead Architect: Er. Adam Eehan (Founder & CEO, Aira Group of Technology)
 * Co-Pilot: Sovereign AI Aira 3 Ultra (Engine v3.6)
 */

#include <airlys/compositor.h>

void apps_init(void);

/* Application Window Draw Functions */
void app_taskmgr_draw(window_t* win);
void app_search_draw(window_t* win);
void app_term_draw(window_t* win);
void app_files_draw(window_t* win);
void app_files_click(window_t* win, int32_t mx, int32_t my);
void app_settings_draw(window_t* win);
void app_settings_click(window_t* win, int32_t mx, int32_t my);
void app_code_draw(window_t* win);
void app_lockscreen_draw(void);

/* Launch Application Windows */
window_t* app_launch_taskmgr(void);
window_t* app_launch_search(void);
window_t* app_launch_terminal(void);
window_t* app_launch_files(void);
window_t* app_launch_settings(void);
window_t* app_launch_code(void);

#endif /* AIRLYS_APPS_H */
