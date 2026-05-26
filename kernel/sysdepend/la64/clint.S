/* clint.S — MMIO at 0x0200_0000 */
.section .text

.macro LOAD_PHYS dst, hi20, lo12, lo12_2=0
    lui  \dst, \hi20
    addi \dst, \dst, \lo12
    .if \lo12_2
    addi \dst, \dst, \lo12_2
    .endif
    slli \dst, \dst, 32
    srli \dst, \dst, 32
.endm

.globl tk2_clint_read_mtime
.globl tk2_clint_read_mtimecmp
.globl tk2_clint_write_mtimecmp

tk2_clint_read_mtime:
    LOAD_PHYS t0, 0x200b, 2047, 2041
    ld   a0, 0(t0)
    ret

tk2_clint_read_mtimecmp:
    LOAD_PHYS t0, 0x2004, 0
    slli t1, a0, 3
    add  t0, t0, t1
    ld   a0, 0(t0)
    ret

tk2_clint_write_mtimecmp:
    LOAD_PHYS t0, 0x2004, 0
    slli t1, a0, 3
    add  t0, t0, t1
    sd   a1, 0(t0)
    ret
