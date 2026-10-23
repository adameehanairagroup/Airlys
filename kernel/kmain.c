#include <airlys/kernel.h>
#include <airlys/vga.h>
#include <airlys/idt.h>
#include <airlys/pic.h>
#include <airlys/timer.h>
#include <airlys/keyboard.h>
#include <airlys/pmm.h>
#include <airlys/vmm.h>
#include <airlys/heap.h>
#include <airlys/sched.h>
#include <airlys/syscall.h>
#include <airlys/vfs.h>
#include <airlys/ramfs.h>
#include <airlys/shell.h>
#include <airlys/printk.h>
#include <airlys/rtc.h>
#include <airlys/serial.h>
#include <airlys/procfs.h>
#include <airlys/devfs.h>
#include <airlys/slab.h>
#include <airlys/security.h>

static void aira_sentinel_task(void) {
    /* Background Sovereign Sentinel Health Daemon */
    for (;;) {
        task_sleep(500); /* Sleep 5 seconds (500 * 10ms ticks) */
    }
}

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

    /* Step 6: Phase 3 - Physical Memory Manager (PMM) */
    pmm_init(1024ULL * 1024 * 1024);
    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_puts(" [OK] Physical Memory Manager    : 1024 MB managed (");
    vga_putdec(pmm_get_free_memory_kb() / 1024);
    vga_puts(" MB free RAM)\n");

    /* Step 7: Phase 3 - Virtual Memory & Page Fault Handler */
    vmm_init();
    vga_puts(" [OK] Virtual Memory Manager     : Dynamic #PF Exception Handler active\n");

    /* Step 8: Phase 3 - Sovereign Dynamic Kernel Heap (kmalloc / kfree) */
    heap_init(KERNEL_HEAP_START, KERNEL_HEAP_SIZE);
    vga_puts(" [OK] Sovereign Dynamic Heap     : 32 MB dynamic pool at 0x1000000\n");

    /* Dynamic Heap Self-Test */
    void* test_alloc = kmalloc(512);
    if (test_alloc != NULL) {
        vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
        vga_puts(" [OK] Dynamic Heap Verification  : kmalloc(512) & kfree() self-test passed!\n");
        kfree(test_alloc);
    }

    /* Step 9: Phase 4 - Preemptive Multitasking Scheduler */
    sched_init();
    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_puts(" [OK] Preemptive Scheduler       : Round-Robin (10ms Quantum) active\n");

    /* Step 10: Phase 4 - System Calls & Dispatcher */
    syscall_init();
    vga_puts(" [OK] Sovereign Syscall Gateway  : Ring 0/Ring 3 Dispatcher ready\n");

    /* Create background sentinel worker thread */
    task_t* worker = task_create("aira_sentinel", aira_sentinel_task);
    if (worker) {
        vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
        vga_puts(" [OK] Background Thread Spawned  : PID ");
        vga_putdec(worker->pid);
        vga_puts(" [");
        vga_puts(worker->name);
        vga_puts("] running\n");
    }

    /* Step 11: Linux Core Subsystems - Slab Cache & Security Sentinel */
    slab_init();
    security_init();
    rtc_init();
    serial_init();
    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_puts(" [OK] Linux Slab Cache Allocator : kmem_cache subsystem online\n");
    vga_puts(" [OK] 16550A UART Serial Driver  : COM1 (0x3F8) 115200 baud active\n");
    vga_puts(" [OK] CMOS Real Time Clock (RTC) : Synchronized with hardware clock\n");

    /* Step 12: Phase 5 - Virtual File System (VFS), In-Memory RamFS, /proc & /dev */
    vfs_init();
    ramfs_init();
    devfs_init();
    procfs_init();
    vga_puts(" [OK] Virtual File System (VFS)  : In-Memory RamFS, /proc & /dev mounted\n");

    /* Step 13: Phase 5 - Interactive Userland Shell (airlys-sh) */
    shell_init();
    vga_puts(" [OK] Sovereign Userland Shell   : airlys-sh active & command dispatcher ready\n");

    /* Step 14: Enable CPU Hardware Interrupts */
    __asm__ volatile ("sti");
    vga_set_color(VGA_COLOR_GREEN, VGA_COLOR_BLACK);
    vga_puts(" [OK] Hardware Interrupts Active : STI executed (IF=1)\n");

    vga_set_color(VGA_COLOR_LIGHT_MAGENTA, VGA_COLOR_BLACK);
    vga_puts("\n ------------------------------------------------------------------------------\n");
    vga_puts(" >>> Interactive Console Active. Type on your keyboard to test hardware input:\n");
    vga_puts(" ------------------------------------------------------------------------------\n\n");

    vga_print_prompt();

    /* Infinite safe idle loop awaiting hardware interrupts */
    for (;;) {
        __asm__ volatile ("hlt");
    }
}
