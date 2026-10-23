#ifndef AIRLYS_VMM_H
#define AIRLYS_VMM_H

#include <airlys/types.h>
#include <airlys/idt.h>

/* Page Table Entry Flags (x86_64) */
#define PAGE_PRESENT        (1ULL << 0)
#define PAGE_WRITABLE       (1ULL << 1)
#define PAGE_USER           (1ULL << 2)
#define PAGE_WRITETHROUGH   (1ULL << 3)
#define PAGE_NOCACHE        (1ULL << 4)
#define PAGE_ACCESSED       (1ULL << 5)
#define PAGE_DIRTY          (1ULL << 6)
#define PAGE_HUGE           (1ULL << 7)
#define PAGE_GLOBAL         (1ULL << 8)
#define PAGE_NOEXECUTE      (1ULL << 63)

/* Read CR2 register (Linear faulting virtual address) */
static inline uintptr_t read_cr2(void) {
    uintptr_t val;
    __asm__ volatile ("mov %%cr2, %0" : "=r"(val));
    return val;
}

/* Read CR3 register (PML4 base address) */
static inline uintptr_t read_cr3(void) {
    uintptr_t val;
    __asm__ volatile ("mov %%cr3, %0" : "=r"(val));
    return val;
}

/* Invalidate TLB entry for a single virtual address */
static inline void invlpg(uintptr_t vaddr) {
    __asm__ volatile ("invlpg (%0)" : : "r"(vaddr) : "memory");
}

/* Initialize Virtual Memory Manager & register #PF handler */
void vmm_init(void);

#endif /* AIRLYS_VMM_H */
