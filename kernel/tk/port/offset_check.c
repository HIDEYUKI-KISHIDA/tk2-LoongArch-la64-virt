/*
 * Build-time check: offset.h must match this toolchain's TCB layout.
 */
#if defined(M5B_TKERNEL)

#define max_tskid   16
#define max_mtxid   8
#define max_ssyid   4
#define max_ssypri  140
#define max_resid   4
#define TEF_VIRT_A64  1

#include <basic.h>
#include <tk/typedef.h>
#include <sys/queue.h>
#include <sys/str_align.h>

typedef struct svclock {
    QUEUE           wtskq;
    struct svclock *locklist;
} SVCLOCK;

typedef struct t_msg T_MSG;

#ifndef __mtxcb__
#define __mtxcb__
typedef struct mutex_control_block MTXCB;
#endif

typedef struct {
    void *ssp;
    void *uatb;
    INT   lsid;
    UW   *svc_ssp;
} CTXB;

#ifndef __tcb__
#define __tcb__
typedef struct task_control_block TCB;
#endif

#include "timer.h"
#include "winfo.h"
#include "task.h"
#include "offset.h"

typedef char _off_tskid[(__builtin_offsetof(TCB, tskid) == TCB_tskid) ? 1 : -1];
typedef char _off_tskctxb[(__builtin_offsetof(TCB, tskctxb) == TCB_tskctxb) ? 1 : -1];
typedef char _off_ctx_uatb[(__builtin_offsetof(CTXB, uatb) == CTXB_uatb) ? 1 : -1];

#endif /* M5B_TKERNEL */
