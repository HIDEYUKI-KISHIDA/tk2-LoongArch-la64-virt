/*
 * M5 API — M5a shim (tk_sched.c) or M5b official T-Kernel types (tk_port_api.c).
 */
#ifndef TK_M5_API_H
#define TK_M5_API_H

#if defined(M5B_TKERNEL)

#include <tk/tkernel.h>

#ifndef TPRI_SELF
#define TPRI_SELF   TPRI_RUN
#endif

void tk_dispatch_start(void);
ER tk_dis_dsp(void);
ER tk_ena_dsp(void);

#else /* M5a cooperative shim */

#include <stddef.h>
#include <stdint.h>

typedef int8_t   B;
typedef int16_t  H;
typedef int32_t  W;
typedef uint32_t UW;
typedef int32_t  INT;
typedef uint32_t UINT;
typedef INT      ER;
typedef INT      PRI;
typedef INT      ID;
typedef UW       ATR;
typedef void   (*FP)(void);

#define CONST   const
#define E_OK    0
#define E_PAR   (-17)
#define E_ID    (-18)
#define E_LIMIT (-34)
#define E_NOEXS (-42)
#define E_OBJ   (-41)
#define E_CTX   (-25)

#define TA_HLNG     0x00000001U
#define TA_RNG0     0x00000000U
#define TA_TFIFO    0x00000004U
#define TPRI_SELF   0

typedef struct t_ctsk {
    void *exinf;
    ATR   tskatr;
    FP    task;
    PRI   itskpri;
    INT   stksz;
} T_CTSK;

typedef struct {
    ATR mtxatr;
    PRI ceilpri;
} T_MTXCB;

ID tk_cre_tsk(CONST T_CTSK *pk_ctsk);
ER tk_sta_tsk(ID tskid, INT stacd);
ER tk_rot_rdq(PRI tskpri);
void tk_ext_tsk(void);

ID tk_cre_mtx(CONST T_MTXCB *pk_cmtx);
ER tk_loc_mtx(ID mtxid);
ER tk_unl_mtx(ID mtxid);

void tk_dispatch_start(void);

#endif /* M5B_TKERNEL */

#endif /* TK_M5_API_H */
