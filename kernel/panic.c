#include <airlys/kernel.h>
#include <airlys/vga.h>

void kernel_panic(const char* message) {
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_RED);
    vga_clear();
    vga_puts("\n\n");
    vga_puts("  ========================================================================\n");
    vga_puts("  [!] AIRLYS SOVEREIGN KERNEL PANIC\n");
    vga_puts("  ========================================================================\n\n");
    vga_puts("  An unrecoverable system state occurred.\n\n");
    vga_puts("  Reason: ");
    vga_puts(message ? message : "Unknown error");
    vga_puts("\n\n");
    vga_puts("  System Halted. Please reboot the host machine.\n");
    vga_puts("  Architecture: x86_64 Long Mode | Aira Group of Technology\n");
    vga_puts("  ========================================================================\n");

    /* Halt the CPU */
    for (;;) {
        __asm__ volatile ("cli; hlt");
    }
}
