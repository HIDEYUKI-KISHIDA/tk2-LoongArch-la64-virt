/* tk2_loongarch.ld — Rev.6.2 §5 / docs/spec */
OUTPUT_ARCH(loongarch)
ENTRY(_start)

/* Linked in DMW1 cached window; load_elf places image at PA 0x40000000 */
MEMORY {
    RAM (rwx) : ORIGIN = 0x9000000004000000, LENGTH = 256M
}

SECTIONS {
    . = 0x9000000004000000;

    .text : {
        *(.text.init)
        *(.text .text.*)
    } > RAM

    .rodata : { *(.rodata .rodata.*) } > RAM

    .data : {
        __data_start = .;
        *(.data .data.*)
        __data_end = .;
    } > RAM

    .bss : {
        . = ALIGN(8);
        __bss_start = .;
        *(.bss .bss.* COMMON)
        __bss_end = .;
    } > RAM

    .stack (NOLOAD) : {
        . = ALIGN(16);
        _stack_bottom = .;
        . += 0x10000;
        _stack_top = .;
    } > RAM

    .percpu (NOLOAD) : {
        _percpu_data = .;
        . += 0x1000;
    } > RAM

    _end = .;
}
