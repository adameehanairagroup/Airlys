/*
 * Airlys Kernel Entry Point & Long-Mode Trampoline
 * Supports both Multiboot 1 and Multiboot 2 specifications
 * Transitions x86 CPU from 32-bit Protected Mode into 64-bit Long Mode
 * Architecture: x86_64 | Aira Group of Technology
 */

.set ALIGN,    1<<0
.set MEMINFO,  1<<1
.set MB1_FLAGS,    ALIGN | MEMINFO
.set MB1_MAGIC,    0x1BADB002
.set MB1_CHECKSUM, -(MB1_MAGIC + MB1_FLAGS)

/* Multiboot 2 Constants */
.set MB2_MAGIC,    0xE85250D6
.set MB2_ARCH,     0
.set MB2_LENGTH,   (multiboot2_header_end - multiboot2_header)
.set MB2_CHECKSUM, -(MB2_MAGIC + MB2_ARCH + MB2_LENGTH)

/* ---------------- Multiboot Headers ---------------- */
.section .multiboot, "a"
.align 8
multiboot2_header:
    .long MB2_MAGIC
    .long MB2_ARCH
    .long MB2_LENGTH
    .long MB2_CHECKSUM

    /* Entry Address Tag (Type 3) */
    .align 8
    .short 3              /* type = 3 (Entry address) */
    .short 0              /* flags = 0 */
    .long 12              /* size = 12 */
    .long _start          /* entry_addr */
    .long 0               /* padding to 8-byte boundary */

    /* Framebuffer Request Tag (Type 5) - Requests UEFI GOP / VBE native @ 32bpp */
    .align 8
    .short 5              /* type = 5 (Framebuffer) */
    .short 0              /* flags = 0 */
    .long 20              /* size = 20 bytes */
    .long 1920            /* width = 1920 (FHD Native for Acer Nitro V 15) */
    .long 1080            /* height = 1080 (FHD Native for Acer Nitro V 15) */
    .long 32              /* depth = 32bpp */

    /* End tag */
    .align 8
    .short 0
    .short 0
    .long 8
multiboot2_header_end:

.align 4
multiboot1_header:
    .long MB1_MAGIC
    .long MB1_FLAGS
    .long MB1_CHECKSUM

/* ---------------- 32-Bit Entry Point ---------------- */
.section .boot, "ax"
.code32
.global _start
.type _start, @function
_start:
    /* Disable interrupts */
    cli

    /* Initialize stack */
    movl $stack_top, %esp

    /* Save Multiboot magic (EAX) and info pointer (EBX) in non-volatile registers */
    movl %eax, %ebp
    movl %ebx, %esi

    /* Zero out the entire .bss section (clears page tables, stacks, global state) */
    movl $_bss_start, %edi
    movl $_bss_end, %ecx
    subl %edi, %ecx
    shrl $2, %ecx           /* Number of 4-byte dwords */
    xorl %eax, %eax
    rep stosl

    /* Restore Multiboot magic to EDI */
    movl %ebp, %edi

    /* Check if CPU supports CPUID */
    pushfl
    popl %eax
    movl %eax, %ecx
    xorl $(1 << 21), %eax
    pushl %eax
    popfl
    pushfl
    popl %eax
    pushl %ecx
    popfl
    cmpl %ecx, %eax
    je no_cpuid

    /* Check if Long Mode is available (EFER.LME) */
    movl $0x80000000, %eax
    cpuid
    cmpl $0x80000001, %eax
    jb no_long_mode

    movl $0x80000001, %eax
    cpuid
    testl $(1 << 29), %edx
    jz no_long_mode

    /* Build identity page tables for 128GB using 2MB huge pages */
    /* PML4[0] -> PDPT */
    movl $pml4_table, %eax
    movl $pdpt_table, %ebx
    orl $0x03, %ebx         /* Present + Writable */
    movl %ebx, (%eax)
    movl $0, 4(%eax)        /* Upper 32 bits = 0 */

    /* PDPT[0..127] -> 128 Page Directories (128 * 1GB = 128GB) */
    movl $pdpt_table, %eax
    movl $page_dir_table, %ebx
    orl $0x03, %ebx
    movl $0, %ecx
init_pdpt_loop:
    movl %ebx, (%eax, %ecx, 8)
    movl $0, 4(%eax, %ecx, 8)
    addl $4096, %ebx
    incl %ecx
    cmpl $128, %ecx
    jne init_pdpt_loop

    /* Identity map 65536 entries of 2MB = 128GB (0x00000000 .. 0x1FFFFFFFFF) */
    movl $0, %ecx
map_pd_loop:
    movl $0x200000, %eax    /* 2MB */
    mull %ecx               /* edx:eax = ecx * 2MB */
    orl $0x83, %eax         /* Present + Writable + Huge Page (bit 7) */
    movl $page_dir_table, %ebx
    movl %eax, (%ebx, %ecx, 8)
    movl %edx, 4(%ebx, %ecx, 8)
    incl %ecx
    cmpl $65536, %ecx
    jne map_pd_loop

    /* Load CR3 with physical address of PML4 */
    movl $pml4_table, %eax
    movl %eax, %cr3

    /* Enable PAE (Physical Address Extension) in CR4 */
    movl %cr4, %eax
    orl $(1 << 5), %eax
    movl %eax, %cr4

    /* Set Long Mode Enable bit in EFER MSR (0xC0000080) */
    movl $0xC0000080, %ecx
    rdmsr
    orl $(1 << 8), %eax
    wrmsr

    /* Load 64-bit Global Descriptor Table BEFORE enabling paging */
    lgdt (gdt64_pointer)

    /* Enable Paging and Protected Mode in CR0 */
    movl %cr0, %eax
    orl $(1 << 31 | 1 << 0), %eax
    movl %eax, %cr0

    /* Far jump to 64-bit code segment */
    ljmp $0x08, $long_mode_entry

no_cpuid:
no_long_mode:
    /* Write error to VGA and halt */
    movl $0x4F524F45, 0xB8000 /* 'E' 'R' with red background */
    cli
halt_loop:
    hlt
    jmp halt_loop

/* ---------------- 64-Bit Long Mode Entry ---------------- */
.code64
.global long_mode_entry
.type long_mode_entry, @function
long_mode_entry:
    /* Reload data segment registers */
    movw $0x10, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    movw %ax, %ss

    /* Setup 64-bit stack pointer */
    movq $stack_top, %rsp

    /* Zero out RFLAGS */
    pushq $0
    popfq

    /* Call Kernel Main with System V ABI arguments:
       rdi = magic (was in edi)
       rsi = multiboot info addr (was in esi)
    */
    movl %edi, %edi
    movl %esi, %esi

    call kernel_main

    /* Hang if kernel_main ever returns */
    cli
hang_64:
    hlt
    jmp hang_64

/* ---------------- Page Tables (4KB Aligned) ---------------- */
.section .bss
.align 4096
pml4_table:
    .skip 4096
pdpt_table:
    .skip 4096
page_dir_table:
    .skip 524288            /* 128 Page Directories * 4096 = 512 KB = 128GB */

/* Kernel Stack (16KB) */
.align 16
stack_bottom:
    .skip 16384
stack_top:

/* ---------------- 64-Bit GDT ---------------- */
.section .rodata
.align 8
gdt64:
    .quad 0                                 /* Null descriptor */
    .quad (1<<41) | (1<<43) | (1<<44) | (1<<47) | (1<<53) /* 64-bit Code: Readable, Executable, Code/Data, Present, Long Mode */
    .quad (1<<41) | (1<<44) | (1<<47)       /* 64-bit Data: Writable, Code/Data, Present */
gdt64_pointer:
    .short . - gdt64 - 1
    .quad gdt64
