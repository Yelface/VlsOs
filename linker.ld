/* Linker script for VlsOs kernel */
ENTRY(start)

/* Memory layout:
   0x00000000 - 0x000003FF : Interrupt Vector Table (IVT)
   0x00000400 - 0x000004FF : BIOS Data Area
   0x00000500 - 0x00004BFF : Free
   0x00004C00 - 0x0009FBFF : Kernel + Stack (usable by us)
   0x0009FC00 - 0x0009FFFF : EBDA (Extended BIOS Data Area)
   0x000F0000 - 0x000FFFFF : ROM/System BIOS
   0x00100000+           : High memory (above 1MB)
*/

SECTIONS {
    /* Kernel starts at 1MB (0x100000) */
    . = 0x100000;

    /* Multiboot header must be in first 8KB of kernel */
    .multiboot : {
        *(.multiboot)
    }

    /* Code section */
    .text ALIGN(4096) : {
        *(.text)
        *(.text.*)
    }

    /* Read-only data */
    .rodata ALIGN(4096) : {
        *(.rodata)
        *(.rodata.*)
    }

    /* Initialized data */
    .data ALIGN(4096) : {
        *(.data)
        *(.data.*)
    }

    /* Uninitialized data (BSS) */
    .bss : {
        *(COMMON)
        *(.bss)
        *(.bss.*)
    }

    /* Symbol to mark end of kernel */
    _kernel_end = .;

    /* Discard unnecessary ELF sections */
    /DISCARD/ : {
        *(.comment)
        *(.gnu.build.id)
        *(.note.*)
    }
}
