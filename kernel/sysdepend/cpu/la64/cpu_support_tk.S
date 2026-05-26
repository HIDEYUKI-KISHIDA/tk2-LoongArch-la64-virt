#define _in_asm_source_
#include "offset.h"
#include "cpu_attr.h"

#define FRAME_SIZE  TK2_CTX_SIZE
#define CRMD_RUN    0x0c
#define CSR_PRMD    0x01
#define CSR_ERA     0x06

    .section .text
    .globl dispatch_to_schedtsk
    .globl dispatch_entry
    .globl dispatch_saved_ctxtsk
    .globl _tk_ret_int
    .globl timer_handler_startup
    .globl knl_task_start_frame_tk

knl_task_start_frame_tk:
    move    $t0, $a0
    move    $t1, $zero
    li.d    $t2, 16
.Lzero:
    st.d    $zero, $t0, 0
    addi.d  $t0, $t0, 8
    addi.d  $t1, $t1, 1
    blt     $t1, $t2, .Lzero
    st.d    $a1, $a0, TK2_CTX_RA
    st.d    $a1, $a0, TK2_CTX_PC
    addi.d  $t1, $a0, FRAME_SIZE
    st.d    $t1, $a0, TK2_CTX_SP
    li.d    $t2, 0x0c
    st.d    $t2, $a0, TK2_CTX_MSTATUS
    jr      $ra

dispatch_entry:
    csrrd   $t0, 0x00
    li.d    $t1, 4
    nor     $t1, $t1, $t1
    and     $t0, $t0, $t1
    csrwr   $t0, 0x00

    la      $t4, ctxtsk
    ld.d    $t5, $t4, 0
    beqz    $t5, .Lloop
    la      $t6, dispatch_disabled
    st.w    $zero, $t6, 0

    addi.d  $sp, $sp, -FRAME_SIZE
    st.d    $ra,  $sp, TK2_CTX_RA
    addi.d  $t0, $sp, FRAME_SIZE
    st.d    $t0, $sp, TK2_CTX_SP
    st.d    $r23, $sp, TK2_CTX_S0
    st.d    $r24, $sp, TK2_CTX_S1
    st.d    $r25, $sp, TK2_CTX_S2
    st.d    $r26, $sp, TK2_CTX_S3
    st.d    $r27, $sp, TK2_CTX_S4
    st.d    $r28, $sp, TK2_CTX_S5
    st.d    $r29, $sp, TK2_CTX_S6
    st.d    $r30, $sp, TK2_CTX_S7
    st.d    $r31, $sp, TK2_CTX_S8
    st.d    $ra,  $sp, TK2_CTX_PC
    csrrd   $t0, 0x00
    st.d    $t0, $sp, TK2_CTX_MSTATUS
    addi.d  $t6, $t5, TCB_tskctxb
    st.d    $sp, $t6, CTXB_ssp
    st.d    $zero, $t4, 0
    move    $a0, $t5
    bl      dispatch_saved_ctxtsk

.Lloop:
    la      $t6, dispatch_disabled
    st.w    $zero, $t6, 0
    la      $t5, schedtsk
    ld.d    $t5, $t5, 0
    beqz    $t5, .Lwait
    la      $t4, ctxtsk
    st.d    $t5, $t4, 0
    addi.d  $a0, $t5, TCB_tskctxb
    ld.d    $sp, $a0, CTXB_ssp
    beqz    $sp, .Lwait
    ld.d    $t0, $sp, TK2_CTX_PC
    beqz    $t0, .Lwait
    ld.d    $ra,  $sp, TK2_CTX_RA
    ld.d    $r23, $sp, TK2_CTX_S0
    ld.d    $r24, $sp, TK2_CTX_S1
    ld.d    $r25, $sp, TK2_CTX_S2
    ld.d    $r26, $sp, TK2_CTX_S3
    ld.d    $r27, $sp, TK2_CTX_S4
    ld.d    $r28, $sp, TK2_CTX_S5
    ld.d    $r29, $sp, TK2_CTX_S6
    ld.d    $r30, $sp, TK2_CTX_S7
    ld.d    $r31, $sp, TK2_CTX_S8
    /* ertn restores CRMD from PRMD — set PRMD (PLV0|DA|IE), ERA from frame */
    ld.d    $t0, $sp, TK2_CTX_MSTATUS
    li.d    $t1, CRMD_RUN
    or      $t0, $t0, $t1
    csrwr   $t0, CSR_PRMD
    ld.d    $t0, $sp, TK2_CTX_PC
    csrwr   $t0, CSR_ERA
    ld.d    $t0, $sp, TK2_CTX_SP
    move    $sp, $t0
    ertn

.Lwait:
    idle    0
    b       .Lloop

dispatch_to_schedtsk:
    la      $sp, tmp_dispatch_stack + 4096
    la      $t6, dispatch_disabled
    li.w    $t0, 2
    st.w    $t0, $t6, 0
    la      $t4, ctxtsk
    st.d    $zero, $t4, 0
    b       .Lloop

_tk_ret_int:
    b       dispatch_entry

timer_handler_startup:
    addi.d  $sp, $sp, -16
    st.d    $ra, $sp, 0
    bl      timer_handler
    ld.d    $ra, $sp, 0
    addi.d  $sp, $sp, 16
    jr      $ra

    .section .bss
    .align  12
    .globl tmp_dispatch_stack
tmp_dispatch_stack:
    .space  4096
