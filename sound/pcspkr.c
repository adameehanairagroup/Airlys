#include <airlys/sound.h>
#include <airlys/io.h>
#include <airlys/timer.h>

void pcspkr_tone(uint32_t freq_hz) {
    if (freq_hz == 0) {
        pcspkr_stop();
        return;
    }

    uint32_t div = 1193180 / freq_hz;
    if (div == 0) div = 1;

    outb(0x43, 0xB6);
    outb(0x42, (uint8_t)(div & 0xFF));
    outb(0x42, (uint8_t)((div >> 8) & 0xFF));

    uint8_t tmp = inb(0x61);
    if ((tmp & 3) != 3) {
        outb(0x61, tmp | 3);
    }
}

void pcspkr_stop(void) {
    uint8_t tmp = inb(0x61) & 0xFC;
    outb(0x61, tmp);
}

void pcspkr_beep(uint32_t freq_hz, uint32_t duration_ms) {
    pcspkr_tone(freq_hz);
    uint32_t ticks = duration_ms / 10;
    if (ticks == 0) ticks = 1;
    timer_sleep(ticks);
    pcspkr_stop();
}
