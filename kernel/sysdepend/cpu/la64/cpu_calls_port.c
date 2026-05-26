/* cpu_calls_port.c — M5b subset of em1d cpu_calls.c (dispatch only) */

#include "kernel.h"
#include "task.h"
#include "check.h"
#include "cpu_status.h"
#include "cpu_task.h"

SYSCALL ER _tk_dis_dsp(void)
{
    CHECK_CTX(!in_loc());
    dispatch_disabled = DDS_DISABLE;
    return E_OK;
}

SYSCALL ER _tk_ena_dsp(void)
{
    CHECK_CTX(!in_loc());
    dispatch_disabled = DDS_ENABLE;
    if (ctxtsk != schedtsk) {
        dispatch();
    }
    return E_OK;
}
