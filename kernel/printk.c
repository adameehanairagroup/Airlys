#include <airlys/printk.h>
#include <airlys/vga.h>
#include <airlys/string.h>

static char printk_ringbuf[PRINTK_BUF_SIZE];
static size_t ringbuf_pos = 0;

static void ringbuf_putc(char c) {
    printk_ringbuf[ringbuf_pos] = c;
    ringbuf_pos = (ringbuf_pos + 1) % PRINTK_BUF_SIZE;
}

static void print_num(uint64_t val, int base, bool is_signed) {
    if (is_signed && (int64_t)val < 0) {
        vga_putchar('-');
        ringbuf_putc('-');
        val = (uint64_t)(-(int64_t)val);
    }

    char buf[32];
    int i = 0;
    const char digits[] = "0123456789abcdef";

    if (val == 0) {
        buf[i++] = '0';
    } else {
        while (val > 0) {
            buf[i++] = digits[val % base];
            val /= base;
        }
    }

    while (--i >= 0) {
        vga_putchar(buf[i]);
        ringbuf_putc(buf[i]);
    }
}

int printk(const char* fmt, ...) {
    if (!fmt) return 0;

    __builtin_va_list args;
    __builtin_va_start(args, fmt);

    /* Check for Linux log level prefix "\001[0-7]" */
    if (fmt[0] == '\001' && fmt[1] >= '0' && fmt[1] <= '7') {
        char level = fmt[1];
        fmt += 2;

        switch (level) {
            case '0': /* EMERG */
            case '1': /* ALERT */
            case '2': /* CRIT */
            case '3': /* ERR */
                vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
                break;
            case '4': /* WARNING */
                vga_set_color(VGA_COLOR_LIGHT_BROWN, VGA_COLOR_BLACK);
                break;
            case '5': /* NOTICE */
            case '6': /* INFO */
                vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
                break;
            case '7': /* DEBUG */
            default:
                vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
                break;
        }
    }

    int chars_written = 0;

    while (*fmt) {
        if (*fmt == '%') {
            fmt++;
            switch (*fmt) {
                case 's': {
                    const char* s = __builtin_va_arg(args, const char*);
                    if (!s) s = "(null)";
                    while (*s) {
                        vga_putchar(*s);
                        ringbuf_putc(*s);
                        s++;
                        chars_written++;
                    }
                    break;
                }
                case 'd':
                case 'i': {
                    int val = __builtin_va_arg(args, int);
                    print_num((uint64_t)val, 10, true);
                    break;
                }
                case 'u': {
                    unsigned int val = __builtin_va_arg(args, unsigned int);
                    print_num((uint64_t)val, 10, false);
                    break;
                }
                case 'x': {
                    unsigned int val = __builtin_va_arg(args, unsigned int);
                    print_num((uint64_t)val, 16, false);
                    break;
                }
                case 'p': {
                    uintptr_t val = __builtin_va_arg(args, uintptr_t);
                    vga_puts("0x");
                    ringbuf_putc('0');
                    ringbuf_putc('x');
                    print_num((uint64_t)val, 16, false);
                    break;
                }
                case 'c': {
                    char c = (char)__builtin_va_arg(args, int);
                    vga_putchar(c);
                    ringbuf_putc(c);
                    chars_written++;
                    break;
                }
                case '%': {
                    vga_putchar('%');
                    ringbuf_putc('%');
                    chars_written++;
                    break;
                }
                default:
                    vga_putchar('%');
                    vga_putchar(*fmt);
                    ringbuf_putc('%');
                    ringbuf_putc(*fmt);
                    break;
            }
        } else {
            vga_putchar(*fmt);
            ringbuf_putc(*fmt);
            chars_written++;
        }
        fmt++;
    }

    __builtin_va_end(args);
    return chars_written;
}

void printk_dump_dmesg(void) {
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    for (size_t i = 0; i < PRINTK_BUF_SIZE; i++) {
        size_t idx = (ringbuf_pos + i) % PRINTK_BUF_SIZE;
        char c = printk_ringbuf[idx];
        if (c != '\0') {
            vga_putchar(c);
        }
    }
}
