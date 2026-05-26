/*
 * cpudef.h (tk/virt RV64) — register types for tk_get_reg / tk_set_reg API
 */

#ifndef __TK_CPUDEF_H__
#define __TK_CPUDEF_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct t_regs {
    VW  r[31];
    void *lr;
} T_REGS;

typedef struct t_eit {
    void *pc;
    UW   spsr;
    UW   taskmode;
} T_EIT;

typedef struct t_cregs {
    void *ssp;
    void *usp;
    void *uatb;
    UW   lsid;
} T_CREGS;

typedef union {
    UW copDummy;
} T_COPREGS;

#ifdef __cplusplus
}
#endif
#endif
