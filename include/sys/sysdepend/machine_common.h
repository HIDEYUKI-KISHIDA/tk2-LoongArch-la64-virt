/*
 * machine_common.h (overlay) — add virt RV64 to upstream router.
 */

#ifndef __SYS_MACHINE_COMMON_H__
#define __SYS_MACHINE_COMMON_H__

#include_next <sys/sysdepend/machine_common.h>

#ifdef _TEF_VIRT_LA64_
#include <sys/sysdepend/tef_virt_la64/machine_depend.h>
#endif

#endif
