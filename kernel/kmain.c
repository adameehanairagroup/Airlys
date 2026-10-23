#include <airlys/kernel.h>
#include <airlys/vga.h>

void kernel_main(uint64_t magic, uint64_t addr) {
    /* Initialize hardware VGA terminal console */
    vga_init();

    /* Banner & Sovereign Branding */
    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_puts("================================================================================\n");
    vga_puts("     _    ___ ____  _  __   ______  \n");
    vga_puts("    / \\  |_ _|  _ \\| | \\ \\ / / ___| \n");
    vga_puts("   / _ \\  | || |_) | |  \\ V /\\___ \\ \n");
    vga_puts("  / ___ \\ | ||  _ <| |___| |  ___) |\n");
    vga_puts(" /_/   \\_\\___|_| \\_\\_____|_| |____/  SOVEREIGN OS KERNEL v0.1.0\n");
    vga_puts("================================================================================\n");

    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_puts(" [OK] Airlys Kernel Genesis initialized successfully!\n");
    vga_puts(" [OK] Architecture : x86_64 (64-Bit Long Mode)\n");
    vga_puts(" [OK] Creator      : Er. Adam Eehan (Founder & CEO, Aira Group)\n");
    vga_puts(" [OK] Co-Pilot     : Sovereign AI Aira 3 Ultra (Engine v3.6)\n");

    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_puts("--------------------------------------------------------------------------------\n");

    vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_puts(" [*] Bootloader Magic : ");
    vga_puthex(magic);
    vga_puts("\n");
    vga_puts(" [*] Multiboot Info   : ");
    vga_puthex(addr);
    vga_puts("\n");

    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_puts(" [*] Physical VGA Memory Map: 0x000B8000 (80x25 Text Mode)\n");
    vga_puts(" [*] Preemptive Subsystems: Ready for Stage 2 (GDT & IDT Interrupts)\n");

    vga_set_color(VGA_COLOR_LIGHT_MAGENTA, VGA_COLOR_BLACK);
    vga_puts("\n >>> Airlys System Idle Loop Active. CPU Halted awaiting hardware ticks.\n");

    /* Infinite safe idle loop */
    for (;;) {
        __asm__ volatile ("hlt");
    }
}
