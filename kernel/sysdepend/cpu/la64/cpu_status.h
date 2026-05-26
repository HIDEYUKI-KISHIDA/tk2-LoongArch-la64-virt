#ifndef _CPU_STATUS_
#define _CPU_STATUS_

#include <tk/sysdef.h>
#include "cpu_insn.h"
#include "m5b_trace.h"

struct task_control_block;
typedef struct task_control_block TCB;

IMPORT TCB *ctxtsk;
IMPORT TCB *schedtsk;
IMPORT INT dispatch_disabled;

#define BEGIN_CRITICAL_SECTION  { UINT _cpsr_ = disint();
#define END_CRITICAL_SECTION    if ( !m5b_ext_tsk_hold_int ) {                \
                                    if ( !isDI(_cpsr_)                       \
                                         && ctxtsk != schedtsk               \
                                         && !isTaskIndependent()             \
                                         && dispatch_disabled == 0 ) {       \
                                        dispatch();                          \
                                    }                                        \
                                    enaint(_cpsr_);                          \
                                } else {                                     \
                                    (void)_cpsr_;                            \
                                } }

#define BEGIN_DISABLE_INTERRUPT { UINT _cpsr_ = disint();
#define END_DISABLE_INTERRUPT   enaint(_cpsr_); }

#define ENABLE_INTERRUPT        { enaint(0); }
#define DISABLE_INTERRUPT       { disint(); }

#define in_indp()       (isTaskIndependent() || ctxtsk == NULL)
#define in_ddsp()       (dispatch_disabled || in_indp() || isDI(get_mstatus()))
#define in_loc()        (in_indp())
#define in_qtsk()       (ctxtsk != NULL && ctxtsk->sysmode > ctxtsk->isysmode)

#define dispatch_request()

typedef struct {
    void *ssp;
    void *uatb;
    INT   lsid;
    UW *svc_ssp;
} CTXB;

IMPORT ATR available_cop;

static inline UINT get_mstatus(void)
{
    UINT crmd;
    asm volatile("csrrd %0, %1" : "=r"(crmd) : "i"(0x00));
    return crmd;
}

void request_tex(TCB *tcb);

Inline void dispatch(void)
{
    extern void dispatch_entry(void);
    dispatch_entry();
}

extern void rv64_force_dispatch(void);

#define force_dispatch()  rv64_force_dispatch()

#endif
