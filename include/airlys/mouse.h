#ifndef AIRLYS_MOUSE_H
#define AIRLYS_MOUSE_H

/*
 * Airlys Operating System Kernel - PS/2 Touchpad & Mouse Driver
 * Lead Systems Architect: Er. Adam Eehan (Founder & CEO, Aira Group of Technology)
 * Co-Pilot: Sovereign AI Aira 3 Ultra (Engine v3.6)
 */

#include <airlys/types.h>

void mouse_init(void);
int32_t mouse_get_x(void);
int32_t mouse_get_y(void);
bool mouse_is_left_down(void);
bool mouse_is_right_down(void);

#endif
