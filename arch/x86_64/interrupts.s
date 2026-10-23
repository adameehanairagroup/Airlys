/*
 * Airlys 64-Bit Interrupt Service Routine (ISR) & IRQ Stubs
 * Architecture: x86_64 Long Mode
 * Aira Group of Technology
 */

.code64
.section .text

.extern isr_handler
.extern irq_handler

/* Macro for ISRs that do NOT push an error code (we push dummy 0) */
.macro ISR_NOERRCODE num
.global isr\num
isr\num:
    pushq $0                /* Dummy error code */
    pushq $\num             /* Interrupt number */
    jmp isr_common_stub
.endm

/* Macro for ISRs that DO push an error code by hardware */
.macro ISR_ERRCODE num
.global isr\num
isr\num:
    pushq $\num             /* Interrupt number */
    jmp isr_common_stub
.endm

/* Macro for IRQ stubs (remapped to 32..47) */
.macro IRQ_STUB irq_num, int_num
.global irq\irq_num
irq\irq_num:
    pushq $0                /* Dummy error code */
    pushq $\int_num         /* Interrupt number (32 + irq_num) */
    jmp irq_common_stub
.endm

/* CPU Exceptions (0-31) */
ISR_NOERRCODE 0
ISR_NOERRCODE 1
ISR_NOERRCODE 2
ISR_NOERRCODE 3
ISR_NOERRCODE 4
ISR_NOERRCODE 5
ISR_NOERRCODE 6
ISR_NOERRCODE 7
ISR_ERRCODE   8
ISR_NOERRCODE 9
ISR_ERRCODE   10
ISR_ERRCODE   11
ISR_ERRCODE   12
ISR_ERRCODE   13
ISR_ERRCODE   14
ISR_NOERRCODE 15
ISR_NOERRCODE 16
ISR_ERRCODE   17
ISR_NOERRCODE 18
ISR_NOERRCODE 19
ISR_NOERRCODE 20
ISR_ERRCODE   21
ISR_NOERRCODE 22
ISR_NOERRCODE 23
ISR_NOERRCODE 24
ISR_NOERRCODE 25
ISR_NOERRCODE 26
ISR_NOERRCODE 27
ISR_NOERRCODE 28
ISR_ERRCODE   29
ISR_ERRCODE   30
ISR_NOERRCODE 31

/* Hardware IRQs (0-15 mapped to 32-47) */
IRQ_STUB 0,  32
IRQ_STUB 1,  33
IRQ_STUB 2,  34
IRQ_STUB 3,  35
IRQ_STUB 4,  36
IRQ_STUB 5,  37
IRQ_STUB 6,  38
IRQ_STUB 7,  39
IRQ_STUB 8,  40
IRQ_STUB 9,  41
IRQ_STUB 10, 42
IRQ_STUB 11, 43
IRQ_STUB 12, 44
IRQ_STUB 13, 45
IRQ_STUB 14, 46
IRQ_STUB 15, 47

/* Common ISR stub for exceptions */
isr_common_stub:
    /* Push general purpose registers */
    pushq %rax
    pushq %rbx
    pushq %rcx
    pushq %rdx
    pushq %rsi
    pushq %rdi
    pushq %rbp
    pushq %r8
    pushq %r9
    pushq %r10
    pushq %r11
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15

    /* First argument in System V AMD64 ABI is %rdi (pointer to registers_t) */
    movq %rsp, %rdi
    call isr_handler

    /* Restore registers */
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %r11
    popq %r10
    popq %r9
    popq %r8
    popq %rbp
    popq %rdi
    popq %rsi
    popq %rdx
    popq %rcx
    popq %rbx
    popq %rax

    /* Clean up interrupt number and error code */
    addq $16, %rsp
    iretq

/* Common IRQ stub for hardware interrupts */
irq_common_stub:
    pushq %rax
    pushq %rbx
    pushq %rcx
    pushq %rdx
    pushq %rsi
    pushq %rdi
    pushq %rbp
    pushq %r8
    pushq %r9
    pushq %r10
    pushq %r11
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15

    movq %rsp, %rdi
    call irq_handler

    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %r11
    popq %r10
    popq %r9
    popq %r8
    popq %rbp
    popq %rdi
    popq %rsi
    popq %rdx
    popq %rcx
    popq %rbx
    popq %rax

    addq $16, %rsp
    iretq
