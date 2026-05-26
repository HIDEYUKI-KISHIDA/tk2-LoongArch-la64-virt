#ifndef __TK_ASM_DEPEND_H__
#define __TK_ASM_DEPEND_H__

#define IMASK   #0x80

.macro .ISB reg, val=#0
    isb
.endm

.macro INT_RETURN
    eret
.endm

.macro EXC_RETURN
    eret
.endm

#endif
