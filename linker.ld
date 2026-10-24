/*
 * Airlys Kernel Linker Script (x86_64 ELF)
 * Aira Group of Technology
 */

ENTRY(_start)

SECTIONS
{
    /* Load kernel at 1MB (standard Multiboot load boundary) */
    . = 1M;
    _kernel_start = .;

    /* Multiboot Header & Executable Code */
    .text : ALIGN(4K)
    {
        KEEP(*(.multiboot))
        KEEP(*(.boot))
        *(.text .text.*)
    }

    /* Read-Only Data (GDT, string constants) */
    .rodata : ALIGN(4K)
    {
        *(.rodata .rodata.*)
    }

    /* Initialized Data */
    .data : ALIGN(4K)
    {
        *(.data .data.*)
    }

    /* Uninitialized Data (BSS, Page Tables, Stack) */
    .bss : ALIGN(4K)
    {
        _bss_start = .;
        *(COMMON)
        *(.bss .bss.*)
        _bss_end = .;
    }

    . = ALIGN(4K);
    _kernel_end = .;

    /* Discard unused unwind & debug sections */
    /DISCARD/ :
    {
        *(.eh_frame)
        *(.comment)
    }
}
