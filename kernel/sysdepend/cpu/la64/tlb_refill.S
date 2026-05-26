#define CSR_TLBRBADV  0x89
#define CSR_PGDL      0x19
#define CSR_PGDH      0x1A
#define CSR_TLBRSAVE  0x8B

    .section .text
    .align  4
    .globl  tlb_refill_entry
tlb_refill_entry:
    csrwr   $t0, CSR_TLBRSAVE
    csrrd   $t1, CSR_TLBRBADV
    srli.d  $t0, $t1, 47
    andi    $t0, $t0, 1
    bnez    $t0, 1f
    csrrd   $t0, CSR_PGDL
    b       2f
1:
    csrrd   $t0, CSR_PGDH
2:
    lddir   $t0, $t0, 3
    lddir   $t0, $t0, 2
    lddir   $t0, $t0, 1
    ldpte   $t0, 0
    ldpte   $t0, 1
    tlbfill
    csrrd   $t0, CSR_TLBRSAVE
    ertn
