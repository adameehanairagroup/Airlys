#include <airlys/idt.h>
#include <airlys/vga.h>
#include <airlys/kernel.h>
#include <airlys/pic.h>

static idt_entry_t idt_entries[256];
static idt_ptr_t   idt_ptr;
static isr_handler_t interrupt_handlers[256];

/* External assembly ISR declarations */
extern void isr0(void);
extern void isr1(void);
extern void isr2(void);
extern void isr3(void);
extern void isr4(void);
extern void isr5(void);
extern void isr6(void);
extern void isr7(void);
extern void isr8(void);
extern void isr9(void);
extern void isr10(void);
extern void isr11(void);
extern void isr12(void);
extern void isr13(void);
extern void isr14(void);
extern void isr15(void);
extern void isr16(void);
extern void isr17(void);
extern void isr18(void);
extern void isr19(void);
extern void isr20(void);
extern void isr21(void);
extern void isr22(void);
extern void isr23(void);
extern void isr24(void);
extern void isr25(void);
extern void isr26(void);
extern void isr27(void);
extern void isr28(void);
extern void isr29(void);
extern void isr30(void);
extern void isr31(void);

/* External assembly IRQ declarations */
extern void irq0(void);
extern void irq1(void);
extern void irq2(void);
extern void irq3(void);
extern void irq4(void);
extern void irq5(void);
extern void irq6(void);
extern void irq7(void);
extern void irq8(void);
extern void irq9(void);
extern void irq10(void);
extern void irq11(void);
extern void irq12(void);
extern void irq13(void);
extern void irq14(void);
extern void irq15(void);

static const char* exception_messages[32] = {
    "Division By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Into Detected Overflow",
    "Out of Bounds",
    "Invalid Opcode",
    "No Coprocessor",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Bad TSS",
    "Segment Not Present",
    "Stack Fault",
    "General Protection Fault",
    "Page Fault",
    "Unknown Interrupt",
    "Coprocessor Fault",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating-Point Exception",
    "Virtualization Exception",
    "Control Protection Exception",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Hypervisor Injection Exception",
    "VMM Communication Exception",
    "Security Exception",
    "Reserved"
};

void idt_set_gate(uint8_t num, uint64_t base, uint16_t sel, uint8_t flags) {
    idt_entries[num].offset_low  = (uint16_t)(base & 0xFFFF);
    idt_entries[num].selector    = sel;
    idt_entries[num].ist         = 0;
    idt_entries[num].type_attr   = flags;
    idt_entries[num].offset_mid  = (uint16_t)((base >> 16) & 0xFFFF);
    idt_entries[num].offset_high = (uint32_t)((base >> 32) & 0xFFFFFFFF);
    idt_entries[num].zero        = 0;
}

void register_interrupt_handler(uint8_t n, isr_handler_t handler) {
    interrupt_handlers[n] = handler;
}

void idt_init(void) {
    idt_ptr.limit = sizeof(idt_entries) - 1;
    idt_ptr.base  = (uint64_t)&idt_entries;

    /* Zero out all handlers */
    for (int i = 0; i < 256; i++) {
        idt_set_gate((uint8_t)i, 0, 0, 0);
        interrupt_handlers[i] = NULL;
    }

    /* 0x8E = Present (0x80) | 64-bit Interrupt Gate (0x0E) */
    uint8_t flags = 0x8E;
    uint16_t cs   = 0x08; /* Kernel 64-bit code segment */

    /* Install CPU Exceptions */
    idt_set_gate(0,  (uint64_t)isr0,  cs, flags);
    idt_set_gate(1,  (uint64_t)isr1,  cs, flags);
    idt_set_gate(2,  (uint64_t)isr2,  cs, flags);
    idt_set_gate(3,  (uint64_t)isr3,  cs, flags);
    idt_set_gate(4,  (uint64_t)isr4,  cs, flags);
    idt_set_gate(5,  (uint64_t)isr5,  cs, flags);
    idt_set_gate(6,  (uint64_t)isr6,  cs, flags);
    idt_set_gate(7,  (uint64_t)isr7,  cs, flags);
    idt_set_gate(8,  (uint64_t)isr8,  cs, flags);
    idt_set_gate(9,  (uint64_t)isr9,  cs, flags);
    idt_set_gate(10, (uint64_t)isr10, cs, flags);
    idt_set_gate(11, (uint64_t)isr11, cs, flags);
    idt_set_gate(12, (uint64_t)isr12, cs, flags);
    idt_set_gate(13, (uint64_t)isr13, cs, flags);
    idt_set_gate(14, (uint64_t)isr14, cs, flags);
    idt_set_gate(15, (uint64_t)isr15, cs, flags);
    idt_set_gate(16, (uint64_t)isr16, cs, flags);
    idt_set_gate(17, (uint64_t)isr17, cs, flags);
    idt_set_gate(18, (uint64_t)isr18, cs, flags);
    idt_set_gate(19, (uint64_t)isr19, cs, flags);
    idt_set_gate(20, (uint64_t)isr20, cs, flags);
    idt_set_gate(21, (uint64_t)isr21, cs, flags);
    idt_set_gate(22, (uint64_t)isr22, cs, flags);
    idt_set_gate(23, (uint64_t)isr23, cs, flags);
    idt_set_gate(24, (uint64_t)isr24, cs, flags);
    idt_set_gate(25, (uint64_t)isr25, cs, flags);
    idt_set_gate(26, (uint64_t)isr26, cs, flags);
    idt_set_gate(27, (uint64_t)isr27, cs, flags);
    idt_set_gate(28, (uint64_t)isr28, cs, flags);
    idt_set_gate(29, (uint64_t)isr29, cs, flags);
    idt_set_gate(30, (uint64_t)isr30, cs, flags);
    idt_set_gate(31, (uint64_t)isr31, cs, flags);

    /* Install Hardware IRQs (32..47) */
    idt_set_gate(32, (uint64_t)irq0,  cs, flags);
    idt_set_gate(33, (uint64_t)irq1,  cs, flags);
    idt_set_gate(34, (uint64_t)irq2,  cs, flags);
    idt_set_gate(35, (uint64_t)irq3,  cs, flags);
    idt_set_gate(36, (uint64_t)irq4,  cs, flags);
    idt_set_gate(37, (uint64_t)irq5,  cs, flags);
    idt_set_gate(38, (uint64_t)irq6,  cs, flags);
    idt_set_gate(39, (uint64_t)irq7,  cs, flags);
    idt_set_gate(40, (uint64_t)irq8,  cs, flags);
    idt_set_gate(41, (uint64_t)irq9,  cs, flags);
    idt_set_gate(42, (uint64_t)irq10, cs, flags);
    idt_set_gate(43, (uint64_t)irq11, cs, flags);
    idt_set_gate(44, (uint64_t)irq12, cs, flags);
    idt_set_gate(45, (uint64_t)irq13, cs, flags);
    idt_set_gate(46, (uint64_t)irq14, cs, flags);
    idt_set_gate(47, (uint64_t)irq15, cs, flags);

    /* Load IDTR register into the CPU */
    __asm__ volatile ("lidt %0" : : "m"(idt_ptr));
}

void isr_handler(registers_t* regs) {
    if (regs->int_no < 32) {
        vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
        vga_puts("\n [CPU EXCEPTION] Vector: ");
        vga_putdec(regs->int_no);
        vga_puts(" -> ");
        vga_puts(exception_messages[regs->int_no]);
        vga_puts(" | ErrCode: ");
        vga_puthex(regs->err_code);
        vga_puts(" | RIP: ");
        vga_puthex(regs->rip);
        vga_puts("\n");

        kernel_panic(exception_messages[regs->int_no]);
    }
}

void irq_handler(registers_t* regs) {
    /* If a custom driver handler is registered, call it */
    if (interrupt_handlers[regs->int_no] != NULL) {
        isr_handler_t handler = interrupt_handlers[regs->int_no];
        handler(regs);
    }

    /* Send End Of Interrupt signal to 8259 PIC */
    pic_send_eoi((uint8_t)(regs->int_no - 32));
}
