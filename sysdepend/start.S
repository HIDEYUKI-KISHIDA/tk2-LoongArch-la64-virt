#define LOONGARCH_CSR_CRMD   0x0
#define LOONGARCH_CSR_CPUID  0x20

    .section .text.init
    .globl _start
    .extern la64_dmw_init
_start:
    csrrd   $t0, LOONGARCH_CSR_CRMD
    li.d    $t1, 4              /* CRMD.IE bit */
    nor     $t1, $t1, $t1      /* ~4 — gas cannot encode ~(1<<2) in li.d */
    and     $t0, $t0, $t1
    csrwr   $t0, LOONGARCH_CSR_CRMD

    csrrd   $t0, LOONGARCH_CSR_CPUID
    bnez    $t0, .Lhalt

    bl      la64_dmw_init

    /* QEMU -kernel jumps to PA; switch to linked VA (DMW1) like Linux head.S */
    la.abs  $t0, .Lvirt
    jirl    $zero, $t0, 0

.Lvirt:
    la      $sp, _stack_top
    la      $tp, _percpu_data

    la      $t0, __bss_start
    la      $t1, __bss_end
.Lbss_clear:
    bgeu    $t0, $t1, .Lbss_done
    st.d    $zero, $t0, 0
    addi.d  $t0, $t0, 8
    b       .Lbss_clear
.Lbss_done:

    bl      exception_vector_init
    bl      main

.Lhalt:
    idle    0
    b       .Lhalt
