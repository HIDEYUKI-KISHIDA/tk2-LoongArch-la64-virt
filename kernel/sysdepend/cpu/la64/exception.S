    .section .text
    .align  4
    .globl  trap_entry
    .globl  exception_entry
    .extern irq_handler
    .extern kernel_sync_exception_handler

trap_entry:
exception_entry:
    csrrd   $t0, 0x00
    li.d    $t1, 4
    nor     $t1, $t1, $t1
    and     $t0, $t0, $t1
    csrwr   $t0, 0x00

    addi.d  $sp, $sp, -288
    st.d    $r1,  $sp,  0*8
    st.d    $r2,  $sp,  1*8
    st.d    $r3,  $sp,  2*8
    st.d    $r4,  $sp,  3*8
    st.d    $r5,  $sp,  4*8
    st.d    $r6,  $sp,  5*8
    st.d    $r7,  $sp,  6*8
    st.d    $r8,  $sp,  7*8
    st.d    $r9,  $sp,  8*8
    st.d    $r10, $sp,  9*8
    st.d    $r11, $sp, 10*8
    st.d    $r12, $sp, 11*8
    st.d    $r13, $sp, 12*8
    st.d    $r14, $sp, 13*8
    st.d    $r15, $sp, 14*8
    st.d    $r16, $sp, 15*8
    st.d    $r17, $sp, 16*8
    st.d    $r18, $sp, 17*8
    st.d    $r19, $sp, 18*8
    st.d    $r20, $sp, 19*8
    st.d    $r21, $sp, 20*8
    st.d    $r22, $sp, 21*8
    st.d    $r23, $sp, 22*8
    st.d    $r24, $sp, 23*8
    st.d    $r25, $sp, 24*8
    st.d    $r26, $sp, 25*8
    st.d    $r27, $sp, 26*8
    st.d    $r28, $sp, 27*8
    st.d    $r29, $sp, 28*8
    st.d    $r30, $sp, 29*8
    st.d    $r31, $sp, 30*8
    addi.d  $t0, $sp, 288
    st.d    $t0, $sp, 2*8
    csrrd   $t0, 0x06
    st.d    $t0, $sp, 31*8
    csrrd   $t0, 0x00
    st.d    $t0, $sp, 32*8
    csrrd   $t0, 0x05
    st.d    $t0, $sp, 34*8

    csrrd   $t0, 0x05
    li.d    $t1, 1 << 11
    and     $t0, $t0, $t1
    beqz    $t0, .Lsync_trap

    bl      irq_handler

    ld.d    $t0, $sp, 31*8
    csrwr   $t0, 0x06
    ld.d    $t0, $sp, 32*8
    ori     $t0, $t0, 0x0c
    csrwr   $t0, 0x01
    ld.d    $r1,  $sp,  0*8
    ld.d    $r4,  $sp,  3*8
    ld.d    $r5,  $sp,  4*8
    ld.d    $r6,  $sp,  5*8
    ld.d    $r7,  $sp,  6*8
    ld.d    $r8,  $sp,  7*8
    ld.d    $r9,  $sp,  8*8
    ld.d    $r10, $sp,  9*8
    ld.d    $r11, $sp, 10*8
    ld.d    $r12, $sp, 11*8
    ld.d    $r13, $sp, 12*8
    ld.d    $r14, $sp, 13*8
    ld.d    $r15, $sp, 14*8
    ld.d    $r16, $sp, 15*8
    ld.d    $r17, $sp, 16*8
    ld.d    $r18, $sp, 17*8
    ld.d    $r19, $sp, 18*8
    ld.d    $r20, $sp, 19*8
    ld.d    $r21, $sp, 20*8
    ld.d    $r22, $sp, 21*8
    ld.d    $r23, $sp, 22*8
    ld.d    $r24, $sp, 23*8
    ld.d    $r25, $sp, 24*8
    ld.d    $r26, $sp, 25*8
    ld.d    $r27, $sp, 26*8
    ld.d    $r28, $sp, 27*8
    ld.d    $r29, $sp, 28*8
    ld.d    $r30, $sp, 29*8
    ld.d    $r31, $sp, 30*8
    ld.d    $r3,  $sp,  2*8
    addi.d  $sp, $sp, 288
    ertn

.Lsync_trap:
    move    $a0, $sp
    bl      kernel_sync_exception_handler
.Lhang:
    idle    0
    b       .Lhang
