#ifndef AIRLYS_TIMER_H
#define AIRLYS_TIMER_H

#include <airlys/types.h>

void timer_init(uint32_t frequency_hz);
uint64_t timer_get_ticks(void);
void timer_sleep(uint32_t ticks);

#endif /* AIRLYS_TIMER_H */
