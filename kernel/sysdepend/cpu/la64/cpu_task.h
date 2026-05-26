#ifndef _CPU_TASK_
#define _CPU_TASK_

#include <basic.h>
#include <stddef.h>
#include <stdint.h>
#include <tk/sysdef.h>
#include <libstr.h>
#include "cpu_insn.h"
#include "cpu_attr.h"
#include "m5b_trace.h"

struct task_control_block;
typedef struct task_control_block TCB;

#define DORMANT_STACK_SIZE      (sizeof(VW) * 8)
#define RESERVE_SSTACK(tskatr)  (TK2_CTX_SIZE)

#define INIT_PSR(rng)           (TK2_INITIAL_MSTATUS_IRQ)
#define INIT_TMF(rng)           (TMF_PPL(rng) | TMF_CPL(rng))

extern void *knl_task_start_frame_tk(void *stack_top, void (*task)(void));

Inline void change_space(void *uatb, INT lsid)
{
    (void)uatb;
    (void)lsid;
}

Inline void setup_context(TCB *tcb)
{
    void *frame;

    frame = knl_task_start_frame_tk((void *)tcb->isstack, (void (*)(void))tcb->task);
    tcb->tskctxb.ssp = frame;
    m5b_log_event(M5B_LOG_DORM, tcb, frame);
    tcb->tskctxb.svc_ssp = 0;
    tcb->tskctxb.uatb = 0;
    tcb->tskctxb.lsid = 0;
}

Inline void setup_stacd(TCB *tcb, INT stacd)
{
    UD *fp = (UD *)tcb->tskctxb.ssp;

    if (fp != NULL) {
        fp[TK2_CTX_S0 / (INT)sizeof(UD)] = (UD)(W)stacd;
        fp[TK2_CTX_S1 / (INT)sizeof(UD)] = (UD)(uintptr_t)tcb->exinf;
    }
}

Inline void cleanup_context(TCB *tcb)
{
    (void)tcb;
}

#endif
