#ifndef AIRLYS_FONT_H
#define AIRLYS_FONT_H

/*
 * Airlys Operating System Kernel - Typography & Font Rendering Engine
 * Lead Architect: Er. Adam Eehan (Founder & CEO, Aira Group of Technology)
 * Co-Pilot: Sovereign AI Aira 3 Ultra (Engine v3.6)
 */

#include <airlys/types.h>

#define FONT_CHAR_WIDTH  8
#define FONT_CHAR_HEIGHT 16

/* Typography API */
void font_draw_char(uint32_t x, uint32_t y, char c, uint32_t color);
void font_draw_string(uint32_t x, uint32_t y, const char* str, uint32_t color);
void font_draw_string_glow(uint32_t x, uint32_t y, const char* str, uint32_t color, uint32_t glow_color);

#endif /* AIRLYS_FONT_H */
