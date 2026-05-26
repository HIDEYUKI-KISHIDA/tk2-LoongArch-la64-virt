OUTPUT_ARCH(loongarch)
ENTRY(_start)

MEMORY {
    RAM (rwx) : ORIGIN = 0x9000000004000000, LENGTH = 256M
}

SECTIONS {
    . = 0x9000000004000000;

    .text.boot : { *(.text.boot) *(.text.init) } > RAM

    .text : { *(.text .text.*) } > RAM
    .rodata : { *(.rodata .rodata.*) } > RAM
    .data : { *(.data .data.*) } > RAM

    .bss : {
        . = ALIGN(8);
        __bss_start = .;
        *(.bss .bss.* COMMON)
        __bss_end = .;
    } > RAM

    .stack (NOLOAD) : ALIGN(16) {
        knl_kernel_stack_bottom = .;
        . += 0x10000;
        knl_kernel_stack_top = .;
    } > RAM
}
