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

static inline void mouse_wait_input(void) {
    uint32_t timeout = 100000;
    while ((inb(MOUSE_PORT_STATUS) & 2) && --timeout > 0) {}
}

static inline void mouse_wait_output(void) {
    uint32_t timeout = 100000;
    while (!(inb(MOUSE_PORT_STATUS) & 1) && --timeout > 0) {}
}

static void mouse_write(uint8_t write) {
    mouse_wait_input();
    outb(MOUSE_PORT_STATUS, MOUSE_CMD_SEND);
    mouse_wait_input();
    outb(MOUSE_PORT_DATA, write);
}

static uint8_t mouse_read(void) {
    mouse_wait_output();
    return inb(MOUSE_PORT_DATA);
}

void mouse_move_relative(int32_t dx, int32_t dy) {
    s_mouse_x += dx;
    s_mouse_y += dy;

    /* Clamp within display bounds */
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
    /* Byte 0 alignment check: Bit 3 must always be 1 in standard PS/2 packet */
    if (s_mouse_cycle == 0 && !(b & 0x08)) {
        return;
    }

    s_mouse_bytes[s_mouse_cycle++] = b;

    if (s_mouse_cycle == 3) {
        s_mouse_cycle = 0;

        uint8_t flags = s_mouse_bytes[0];
        int32_t dx = (int32_t)s_mouse_bytes[1];
        int32_t dy = (int32_t)s_mouse_bytes[2];

        /* Sign extension for 9-bit deltas */
        if (flags & 0x10) dx |= 0xFFFFFF00;
        if (flags & 0x20) dy |= 0xFFFFFF00;

        /* Ignore overflow packets */
        if (flags & 0x40 || flags & 0x80) return;

        /* Invert dy because PS/2 Y is positive upwards, display Y is downwards */
        mouse_move_relative(dx, -dy);

        s_left_button  = (flags & 0x01) != 0;
        s_right_button = (flags & 0x02) != 0;

        /* Dispatch event to window compositor */
        compositor_handle_mouse(s_mouse_x, s_mouse_y, s_left_button, s_right_button);
    }
}

static void mouse_callback(registers_t* regs) {
    (void)regs;
    uint8_t status = inb(MOUSE_PORT_STATUS);
    if (!(status & 0x01)) return; /* No data ready */

    uint8_t b = inb(MOUSE_PORT_DATA);
    mouse_process_byte(b);
}

/* Polling fallback for systems where IRQ 12 is intercepted by UEFI EC */
void mouse_poll(void) {
    uint32_t guard = 16;
    while ((inb(MOUSE_PORT_STATUS) & 0x01) && guard-- > 0) {
        uint8_t b = inb(MOUSE_PORT_DATA);
        mouse_process_byte(b);
    }
}

void mouse_init(void) {
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

    /* Step 4: Set mouse to default sampling rate */
    mouse_write(0xF6);
    (void)mouse_read();

    /* Step 5: Enable streaming / packet reporting */
    mouse_write(0xF4);
    (void)mouse_read();

    /* Step 6: Register Vector 44 handler for IRQ 12 */
    register_interrupt_handler(44, mouse_callback);

    /* Step 7: Unmask PIC IRQ 2 (Cascade) & IRQ 12 (Mouse) */
    pic_unmask_irq(2);
    pic_unmask_irq(12);

    /* Center cursor */
    vbe_screen_t* scr = vbe_get_screen();
    if (scr && scr->width > 0 && scr->height > 0) {
        s_mouse_x = (int32_t)scr->width / 2;
        s_mouse_y = (int32_t)scr->height / 2;
    }
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
