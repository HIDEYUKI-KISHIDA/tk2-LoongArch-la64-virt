#include "kernel.h"

IMPORT INT svc_call_limit;

EXPORT ER ChkCallPLevel(void)
{
    (void)svc_call_limit;
    return E_OK;
}
