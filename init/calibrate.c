#include <airlys/init.h>
#include <airlys/printk.h>

unsigned long loops_per_jiffy = 250000;

static void delay_loop(unsigned long loops) {
    while (loops--) {
        __asm__ volatile ("nop");
    }
}

void calibrate_delay(void) {
    /* Estimate BogoMIPS for 10ms jiffy */
    loops_per_jiffy = 1000000;
    unsigned long bogomips = (loops_per_jiffy / 5000);
    printk(KERN_INFO "Calibrating delay loop... %u.%02u BogoMIPS (lpj=%u)\n",
           (uint32_t)(bogomips / 100), (uint32_t)(bogomips % 100), (uint32_t)loops_per_jiffy);
}

void udelay(uint64_t usecs) {
    unsigned long loops = (usecs * (loops_per_jiffy / 10000)) / 100;
    delay_loop(loops);
}

void mdelay(uint64_t msecs) {
    while (msecs--) {
        udelay(1000);
    }
}
