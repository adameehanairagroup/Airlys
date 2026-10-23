/*
 * Airlys Operating System - x86_64 Context Switch
 * Engineered by Er. Adam Eehan (Founder & CEO, Aira Group of Technology)
 * Co-Engineered with Sovereign AI Aira 3 Ultra (Engine v3.6)
 *
 * Prototype: void task_switch(uintptr_t* prev_rsp, uintptr_t* next_rsp);
 * System V ABI: %rdi = prev_rsp, %rsi = next_rsp
 */

.section .text
.global task_switch
.type task_switch, @function
task_switch:
    /* 1. Save RFLAGS and Callee-saved general purpose registers */
    pushfq
    pushq %rbx
    pushq %rbp
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15

    /* 2. Store current stack pointer in outgoing task structure */
    movq %rsp, (%rdi)

    /* 3. Switch stack pointer to incoming task */
    movq (%rsi), %rsp

    /* 4. Restore Callee-saved registers of incoming task */
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %rbp
    popq %rbx
    popfq

    /* 5. Return jumps to incoming task's instruction pointer (RIP) */
    ret
