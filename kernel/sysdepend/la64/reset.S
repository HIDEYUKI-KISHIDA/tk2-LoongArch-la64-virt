#define CSR_CRMD  0x0
#define CSR_CPUID 0x20

    .section .text.boot
    .globl _start
    .extern board_init
    .extern knl_main
    .extern exception_vector_init
    .extern la64_dmw_init

_start:
    csrrd   $t0, CSR_CRMD
    li.d    $t1, 4
    nor     $t1, $t1, $t1
    and     $t0, $t0, $t1
    csrwr   $t0, CSR_CRMD

    csrrd   $t0, CSR_CPUID
    bnez    $t0, .Lhalt

    bl      la64_dmw_init

    la.abs  $t0, .Lvirt
    jirl    $zero, $t0, 0

.Lvirt:
    la      $sp, knl_kernel_stack_top
    la      $tp, _percpu_data

    la      $t0, __bss_start
    la      $t1, __bss_end
.Lbss:
    bgeu    $t0, $t1, .Lbss_done
    st.d    $zero, $t0, 0
    addi.d  $t0, $t0, 8
    b       .Lbss
.Lbss_done:

    bl      exception_vector_init
    bl      board_init
    bl      knl_main

.Lhalt:
    idle    0
    b       .Lhalt

    .section .bss
    .align  12
    .globl  _percpu_data
_percpu_data:
    .space  0x1000
