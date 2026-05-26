#ifndef CPU_ATTR_H
#define CPU_ATTR_H

#ifndef __ASSEMBLER__
#include <stdint.h>
#endif

#define TK2_CTX_SIZE           128

#define TK2_CTX_RA              0
#define TK2_CTX_SP              8
#define TK2_CTX_S0             16
#define TK2_CTX_S1             24
#define TK2_CTX_S2             32
#define TK2_CTX_S3             40
#define TK2_CTX_S4             48
#define TK2_CTX_S5             56
#define TK2_CTX_S6             64
#define TK2_CTX_S7             72
#define TK2_CTX_S8             80
#define TK2_CTX_S9             88
#define TK2_CTX_S10            96
#define TK2_CTX_S11           104
#define TK2_CTX_PC            112   /* ERA */
#define TK2_CTX_MSTATUS       120   /* CRMD */

#define TK2_CTX_X30           TK2_CTX_RA
#define TK2_CTX_ELR           TK2_CTX_PC
#define TK2_CTX_SPSR          TK2_CTX_MSTATUS
#define TK2_CTX_SP_KSP        TK2_CTX_SP

/* PLV0 | DA | IE */
#define TK2_INITIAL_MSTATUS_IRQ   (0x08UL | (1UL << 2))

#ifndef __ASSEMBLER__
typedef struct {
    void *ssp;
} TSKCTXB;
#endif

#endif
