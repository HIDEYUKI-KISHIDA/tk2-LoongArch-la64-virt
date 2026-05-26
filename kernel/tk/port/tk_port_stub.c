/* tk_port_stub.c — M5b symbols not in minimal tkernel link set */

#include <tk/errno.h>
#include <tk/typedef.h>

EXPORT ER no_support(void)
{
    return E_RSFN;
}

EXPORT void hook_svc(void) { }
EXPORT void unhook_svc(void) { }
EXPORT void hook_dsp(void) { }
EXPORT void unhook_dsp(void) { }
EXPORT void hook_int(void) { }
EXPORT void unhook_int(void) { }
