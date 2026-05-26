#ifndef __TK_SYSCALL_COMMON_H__
#define __TK_SYSCALL_COMMON_H__

#include_next <tk/sysdepend/syscall_common.h>

#if defined(_TEF_VIRT_LA64_) || TEF_VIRT_LA64
#include <tk/sysdepend/tef_virt_la64/cpuattr.h>
#include <tk/sysdepend/tef_virt_la64/cpudef.h>
#endif

#endif
