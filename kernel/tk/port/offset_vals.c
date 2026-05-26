/*
 * Compiled with aarch64-linux-gnu-gcc (same as kernel).
 * Compiled with -S; Makefile parses .xword values into offset.h.
 */

#define max_tskid   16
#define max_semid   0
#define max_flgid   0
#define max_mbxid   0
#define max_mtxid   8
#define max_mbfid   0
#define max_porid   0
#define max_mplid   0
#define max_mpfid   0
#define max_cycid   0
#define max_almid   0
#define max_ssyid   4
#define max_ssypri  140
#define max_resid   4
#define TEF_VIRT_LA64  1

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

#define OFF_EXPORT(name, member) \
    const unsigned long off_export_##name __attribute__((used)) = \
        (unsigned long)__builtin_offsetof(TCB, member)

#define CTX_EXPORT(name, member) \
    const unsigned long off_export_CTXB_##name __attribute__((used)) = \
        (unsigned long)__builtin_offsetof(CTXB, member)

OFF_EXPORT(tskid, tskid);
OFF_EXPORT(tskatr, tskatr);
OFF_EXPORT(state, state);
OFF_EXPORT(reqdct, reqdct);
OFF_EXPORT(tskctxb, tskctxb);
CTX_EXPORT(uatb, uatb);
CTX_EXPORT(lsid, lsid);
CTX_EXPORT(svc_ssp, svc_ssp);
