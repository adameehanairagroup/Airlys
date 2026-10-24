#include <airlys/mouse.h>
#include <airlys/compositor.h>
#include <airlys/idt.h>
#include <airlys/io.h>
#include <airlys/pic.h>
#include <airlys/vbe.h>

#define MOUSE_PORT_DATA   0x60
#define MOUSE_PORT_STATUS 0x64
#define MOUSE_CMD_SEND    0xD4

static int32_t s_mouse_x = 960;
static int32_t s_mouse_y = 540;
static uint8_t s_mouse_cycle = 0;
static uint8_t s_mouse_bytes[3];
static bool    s_left_button = false;
static bool    s_right_button = false;
static bool    s_mouse_present = false;

static inline void mouse_wait_input(void) {
    uint32_t timeout = 50000;
    while ((inb(MOUSE_PORT_STATUS) & 2) && --timeout > 0) {}
}

static inline bool mouse_wait_output(void) {
    uint32_t timeout = 50000;
    while (!(inb(MOUSE_PORT_STATUS) & 1) && --timeout > 0) {}
    return timeout > 0;
}

static void mouse_write(uint8_t write) {
    mouse_wait_input();
    outb(MOUSE_PORT_STATUS, MOUSE_CMD_SEND);
    mouse_wait_input();
    outb(MOUSE_PORT_DATA, write);
}

static uint8_t mouse_read(void) {
    if (!mouse_wait_output()) return 0xFF;
    return inb(MOUSE_PORT_DATA);
}

void mouse_move_relative(int32_t dx, int32_t dy) {
    s_mouse_x += dx;
    s_mouse_y += dy;

    /* Clamp within physical display bounds */
    vbe_screen_t* scr = vbe_get_screen();
    if (scr && scr->width > 0 && scr->height > 0) {
        if (s_mouse_x < 0) s_mouse_x = 0;
        if (s_mouse_x >= (int32_t)scr->width) s_mouse_x = (int32_t)scr->width - 1;
        if (s_mouse_y < 0) s_mouse_y = 0;
        if (s_mouse_y >= (int32_t)scr->height) s_mouse_y = (int32_t)scr->height - 1;
    }
}

void mouse_set_buttons(bool left, bool right) {
    s_left_button = left;
    s_right_button = right;
    compositor_handle_mouse(s_mouse_x, s_mouse_y, s_left_button, s_right_button);
}

static void mouse_process_byte(uint8_t b) {
    /* Strict PS/2 byte 0 validation to eliminate floating bus 0xFF/0x00 noise */
    if (s_mouse_cycle == 0) {
        /* Bit 3 MUST be 1; Bit 6 & 7 (overflow) MUST be 0; cannot be 0xFF or 0x00 */
        if ((b & 0x08) == 0 || (b & 0xC0) != 0 || b == 0xFF || b == 0x00) {
            return; /* Discard floating noise */
        }
    }

    s_mouse_bytes[s_mouse_cycle++] = b;

    if (s_mouse_cycle == 3) {
        s_mouse_cycle = 0;

        uint8_t flags = s_mouse_bytes[0];
        int32_t dx = (int32_t)s_mouse_bytes[1];
        int32_t dy = (int32_t)s_mouse_bytes[2];

        /* Sanity checks on flags */
        if (flags == 0xFF || flags == 0x00) return;
        if ((flags & 0x08) == 0 || (flags & 0xC0) != 0) return;

        /* Sign extension for 9-bit relative deltas */
        if (flags & 0x10) dx |= 0xFFFFFF00;
        if (flags & 0x20) dy |= 0xFFFFFF00;

        /* Ignore unrealistic jumps (noise suppression) */
        if (dx > 200 || dx < -200 || dy > 200 || dy < -200) return;

        /* Move cursor (PS/2 dy is inverted) */
        mouse_move_relative(dx, -dy);

        s_left_button  = (flags & 0x01) != 0;
        s_right_button = (flags & 0x02) != 0;

        compositor_handle_mouse(s_mouse_x, s_mouse_y, s_left_button, s_right_button);
    }
}

static void mouse_callback(registers_t* regs) {
    (void)regs;
    uint8_t status = inb(MOUSE_PORT_STATUS);
    if (!(status & 0x01)) return;

    uint8_t b = inb(MOUSE_PORT_DATA);
    mouse_process_byte(b);
}

void mouse_poll(void) {
    uint32_t guard = 16;
    while ((inb(MOUSE_PORT_STATUS) & 0x01) && guard-- > 0) {
        uint8_t b = inb(MOUSE_PORT_DATA);
        mouse_process_byte(b);
    }
}

void mouse_init(void) {
    /* Center cursor on 1080p display */
    vbe_screen_t* scr = vbe_get_screen();
    if (scr && scr->width > 0 && scr->height > 0) {
        s_mouse_x = (int32_t)scr->width / 2;
        s_mouse_y = (int32_t)scr->height / 2;
    } else {
        s_mouse_x = 960;
        s_mouse_y = 540;
    }
    s_mouse_cycle = 0;
    s_left_button = false;
    s_right_button = false;

    /* Step 1: Enable auxiliary mouse device on PS/2 controller */
    mouse_wait_input();
    outb(MOUSE_PORT_STATUS, 0xA8);

    /* Step 2: Read controller configuration byte */
    mouse_wait_input();
    outb(MOUSE_PORT_STATUS, 0x20);
    uint8_t status = mouse_read();

    /* Enable IRQ 12 (bit 1) and clear mouse clock disable (bit 5) */
    status |= (1 << 1);
    status &= ~(1 << 5);

    /* Step 3: Write updated controller configuration byte */
    mouse_wait_input();
    outb(MOUSE_PORT_STATUS, 0x60);
    mouse_wait_input();
    outb(MOUSE_PORT_DATA, status);

    /* Step 4: Reset & Self-test */
    mouse_write(0xFF);
    uint8_t ack = mouse_read();
    if (ack == 0xFA) {
        uint8_t test = mouse_read();
        if (test == 0xAA) {
            s_mouse_present = true;
            (void)mouse_read(); /* Drain device ID */
        }
    }

    /* Step 5: Enable packet streaming */
    mouse_write(0xF4);
    (void)mouse_read();

    /* Step 6: Register Vector 44 handler for IRQ 12 */
    register_interrupt_handler(44, mouse_callback);

    /* Step 7: Unmask PIC IRQ 2 & IRQ 12 */
    pic_unmask_irq(2);
    pic_unmask_irq(12);
}

int32_t mouse_get_x(void) {
    return s_mouse_x;
}

int32_t mouse_get_y(void) {
    return s_mouse_y;
}

bool mouse_is_left_down(void) {
    return s_left_button;
}

bool mouse_is_right_down(void) {
    return s_right_button;
}
