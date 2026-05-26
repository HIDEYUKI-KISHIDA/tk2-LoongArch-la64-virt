#ifndef __SYS_ROMINFO_COMMON_H__
#define __SYS_ROMINFO_COMMON_H__

#include_next <sys/sysdepend/rominfo_common.h>

#if defined(_TEF_VIRT_LA64_) || TEF_VIRT_LA64
#include <sys/sysdepend/tef_virt_la64/rominfo_depend.h>
#endif

#endif
