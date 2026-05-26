/*
 * machine_depend.h (sys/virt RV64 QEMU)
 * Overlay for tk2-aarch64 — not part of upstream tkernel_2.
 */

#ifndef __SYS_MACHINE_DEPEND_H__
#define __SYS_MACHINE_DEPEND_H__

#undef _TEF_VIRT_LA64_
#define _TEF_VIRT_LA64_      1
#define CPU_ARMV8A          1
#define CPU_ARM             1
#define TEF_VIRT_LA64        1
#define ALLOW_MISALIGN      0
#define BIGENDIAN           0
#define VIRTUAL_ADDRESS     1
#define ALLOCA_NOSPT        1

#ifndef _Csym
#define _Csym               0
#endif

#endif
