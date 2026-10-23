#include <airlys/kernel.h>
#include <airlys/vga.h>
#include <airlys/idt.h>
#include <airlys/pic.h>
#include <airlys/timer.h>
#include <airlys/keyboard.h>

void kernel_main(uint64_t magic, uint64_t addr) {
    /* Step 1: Initialize hardware VGA terminal console */
    vga_init();

    /* Banner & Sovereign Branding */
    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_puts("================================================================================\n");
    vga_puts("     _    ___ ____  _  __   ______  \n");
    vga_puts("    / \\  |_ _|  _ \\| | \\ \\ / / ___| \n");
    vga_puts("   / _ \\  | || |_) | |  \\ V /\\___ \\ \n");
    vga_puts("  / ___ \\ | ||  _ <| |___| |  ___) |\n");
    vga_puts(" /_/   \\_\\___|_| \\_\\_____|_| |____/  SOVEREIGN OS KERNEL v0.2.0\n");
    vga_puts("================================================================================\n");

    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_puts(" [OK] Airlys Kernel Architecture : x86_64 Long Mode (64-Bit)\n");
    vga_puts(" [OK] Lead Systems Architect     : Er. Adam Eehan (Founder & CEO, Aira Group)\n");
    vga_puts(" [OK] Engineering Co-Pilot       : Sovereign AI Aira 3 Ultra (Engine v3.6)\n");

    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_puts("--------------------------------------------------------------------------------\n");

    vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_puts(" [*] Bootloader Magic : ");
    vga_puthex(magic);
    vga_puts(" | Multiboot Info: ");
    vga_puthex(addr);
    vga_puts("\n");

    /* Step 2: Initialize 64-bit Interrupt Descriptor Table (IDT) */
    idt_init();
    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_puts(" [OK] CPU IDT Loaded             : 256 Gates (Vectors 0..255 active)\n");

    /* Step 3: Remap 8259 Dual Programmable Interrupt Controllers */
    pic_init();
    vga_puts(" [OK] Dual 8259 PIC Remapped     : Master IRQ 32..39, Slave IRQ 40..47\n");

    /* Step 4: Initialize Programmable Interval Timer (PIT) at 100 Hz */
    timer_init(100);
    vga_puts(" [OK] Programmable Interval Timer: 100 Hz (10ms preemptive tick interval)\n");

    /* Step 5: Initialize PS/2 Keyboard Driver */
    keyboard_init();
    vga_puts(" [OK] PS/2 Hardware Keyboard     : Scan Code Set 1 Driver active\n");

    /* Step 6: Enable CPU Hardware Interrupts */
    __asm__ volatile ("sti");
    vga_set_color(VGA_COLOR_GREEN, VGA_COLOR_BLACK);
    vga_puts(" [OK] Hardware Interrupts Active : STI executed (IF=1)\n");

    vga_set_color(VGA_COLOR_LIGHT_MAGENTA, VGA_COLOR_BLACK);
    vga_puts("\n ------------------------------------------------------------------------------\n");
    vga_puts(" >>> Interactive Console Active. Type on your keyboard to test hardware input:\n");
    vga_puts(" ------------------------------------------------------------------------------\n\n");

    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_puts("airlys> ");

    /* Infinite safe idle loop awaiting hardware interrupts */
    for (;;) {
        __asm__ volatile ("hlt");
    }
}
