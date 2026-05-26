#if defined(M5B_TKERNEL) || defined(TK2_TKERNEL)
#include "kernel.h"
#include "task.h"
#include "cpu_insn.h"

EXPORT MONHDR SaveMonHdr;
EXPORT ATR available_cop;

EXPORT ER cpu_initialize(void)
{
    available_cop = TA_NULL;
    SaveMonHdr.default_hdr = 0;
    return E_OK;
}

EXPORT void cpu_shutdown(void)
{
}

EXPORT void request_tex(TCB *tcb)
{
    if (tcb != 0 && tcb->isysmode == 0) {
        tcb->reqdct = 1;
    }
}
#endif
