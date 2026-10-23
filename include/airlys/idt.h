#ifndef AIRLYS_IDT_H
#define AIRLYS_IDT_H

#include <airlys/types.h>

/* 64-Bit Interrupt Descriptor Table Entry */
typedef struct __attribute__((packed)) {
    uint16_t offset_low;      /* Target address bits 0..15 */
    uint16_t selector;        /* Kernel code segment selector (0x08) */
    uint8_t  ist;             /* Interrupt Stack Table index (bits 0..2) */
    uint8_t  type_attr;       /* Type and attributes (flags) */
    uint16_t offset_mid;      /* Target address bits 16..31 */
    uint32_t offset_high;     /* Target address bits 32..63 */
    uint32_t zero;            /* Reserved (must be zero) */
} idt_entry_t;

/* IDTR register pointer */
typedef struct __attribute__((packed)) {
    uint16_t limit;
    uint64_t base;
} idt_ptr_t;

/* Register state saved on the stack by assembly ISR stubs */
typedef struct __attribute__((packed)) {
    /* Pushed by common assembly stub (reverse order of push) */
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rbp, rdi, rsi, rdx, rcx, rbx, rax;
    
    /* Pushed by individual ISR stub */
    uint64_t int_no;
    uint64_t err_code;

    /* Pushed automatically by CPU hardware on interrupt */
    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;
    uint64_t rsp;
    uint64_t ss;
} registers_t;

typedef void (*isr_handler_t)(registers_t*);

void idt_init(void);
void idt_set_gate(uint8_t num, uint64_t base, uint16_t sel, uint8_t flags);
void register_interrupt_handler(uint8_t n, isr_handler_t handler);

#endif /* AIRLYS_IDT_H */
