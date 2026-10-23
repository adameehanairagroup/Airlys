#include <airlys/vmm.h>
#include <airlys/vga.h>
#include <airlys/kernel.h>

static void page_fault_handler(registers_t* regs) {
    uintptr_t faulting_addr = read_cr2();

    bool present  = (regs->err_code & 0x1) != 0;
    bool write    = (regs->err_code & 0x2) != 0;
    bool user     = (regs->err_code & 0x4) != 0;
    bool reserved = (regs->err_code & 0x8) != 0;
    bool ifetch   = (regs->err_code & 0x10) != 0;

    vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
    vga_puts("\n\n [FATAL] PAGE FAULT (#PF) DETECTED!\n");
    vga_puts(" ----------------------------------------------------\n");
    vga_puts(" Faulting Virtual Address (CR2) : ");
    vga_puthex(faulting_addr);
    vga_puts("\n Error Code                     : ");
    vga_puthex(regs->err_code);
    vga_puts("\n Flags: ");
    if (!present) vga_puts("[Not Present] ");
    else          vga_puts("[Protection Violation] ");
    if (write)    vga_puts("[Write] ");
    else          vga_puts("[Read] ");
    if (user)     vga_puts("[User Ring3] ");
    else          vga_puts("[Kernel Ring0] ");
    if (reserved) vga_puts("[Reserved Bit] ");
    if (ifetch)   vga_puts("[Instruction Fetch] ");
    vga_puts("\n ----------------------------------------------------\n");

    kernel_panic("Kernel unhandled page fault access violation");
}

void vmm_init(void) {
    /* Register Page Fault handler on CPU Exception Vector 14 */
    register_interrupt_handler(14, page_fault_handler);
}
