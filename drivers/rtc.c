#include <airlys/rtc.h>
#include <airlys/io.h>

#define CMOS_ADDRESS 0x70
#define CMOS_DATA    0x71

static uint8_t cmos_read(uint8_t reg) {
    outb(CMOS_ADDRESS, reg);
    return inb(CMOS_DATA);
}

static uint8_t bcd_to_binary(uint8_t bcd) {
    return ((bcd / 16) * 10) + (bcd & 0x0F);
}

void rtc_init(void) {
    /* Initialize and verify RTC availability */
}

rtc_time_t rtc_get_time(void) {
    rtc_time_t t;

    /* Wait if RTC update in progress */
    while (cmos_read(0x0A) & 0x80) {}

    t.second = cmos_read(0x00);
    t.minute = cmos_read(0x02);
    t.hour   = cmos_read(0x04);
    t.day    = cmos_read(0x07);
    t.month  = cmos_read(0x08);
    uint8_t year_byte = cmos_read(0x09);

    uint8_t register_b = cmos_read(0x0B);

    /* Convert BCD to binary if needed */
    if (!(register_b & 0x04)) {
        t.second = bcd_to_binary(t.second);
        t.minute = bcd_to_binary(t.minute);
        t.hour   = bcd_to_binary(t.hour & 0x7F) | (t.hour & 0x80);
        t.day    = bcd_to_binary(t.day);
        t.month  = bcd_to_binary(t.month);
        year_byte = bcd_to_binary(year_byte);
    }

    /* Convert 12 hour clock to 24 hour if necessary */
    if (!(register_b & 0x02) && (t.hour & 0x80)) {
        t.hour = ((t.hour & 0x7F) + 12) % 24;
    }

    t.year = 2000 + year_byte;

    return t;
}
