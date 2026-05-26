    .section .text
    .align  4
    .globl  machine_error_entry
machine_error_entry:
1:
    idle    0
    b       1b
