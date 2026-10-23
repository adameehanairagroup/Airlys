#include <airlys/timer.h>
#include <airlys/idt.h>
#include <airlys/io.h>
#include <airlys/pic.h>

#define PIT_CHANNEL0 0x40
#define PIT_COMMAND  0x43
#define PIT_BASE_FREQ 1193182

static volatile uint64_t timer_ticks = 0;

static void timer_callback(registers_t* regs) {
    (void)regs;
    timer_ticks++;
}

void timer_init(uint32_t frequency_hz) {
    if (frequency_hz == 0) frequency_hz = 100;

    uint32_t divisor = PIT_BASE_FREQ / frequency_hz;
    if (divisor == 0) divisor = 1;
    if (divisor > 65535) divisor = 65535;

    /* Register callback on IRQ0 (Vector 32) */
    register_interrupt_handler(32, timer_callback);

    /* 0x36 = Channel 0, lobyte/hibyte, Mode 3 (Square wave generator), binary 16-bit */
    outb(PIT_COMMAND, 0x36);
    outb(PIT_CHANNEL0, (uint8_t)(divisor & 0xFF));
    outb(PIT_CHANNEL0, (uint8_t)((divisor >> 8) & 0xFF));

    /* Unmask IRQ0 on Master PIC */
    pic_unmask_irq(0);
}

uint64_t timer_get_ticks(void) {
    return timer_ticks;
}

void timer_sleep(uint32_t ticks) {
    uint64_t start = timer_ticks;
    while ((timer_ticks - start) < ticks) {
        __asm__ volatile ("hlt");
    }
}
