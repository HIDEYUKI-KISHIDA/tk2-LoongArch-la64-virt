/*
 * sysdef_depend.h (tk/virt RV64)
 */

#ifndef __TK_SYSDEF_DEPEND_H__
#define __TK_SYSDEF_DEPEND_H__

/* RV64 PSTATE / SPSR (EL1h) */
#define PSR_N               (1ULL << 31)
#define PSR_Z               (1ULL << 30)
#define PSR_C               (1ULL << 29)
#define PSR_V               (1ULL << 28)
#define PSR_D               (1ULL << 9)
#define PSR_A               (1ULL << 8)
#define PSR_I               (1ULL << 7)
#define PSR_F               (1ULL << 6)
#define PSR_M_EL1h          5ULL
#define PSR_M_EL0t          0ULL

#define PSR_EL1h            PSR_M_EL1h
#define PSR_EL0t            PSR_M_EL0t
#define PSR_DI              (PSR_I | PSR_A)

/* Initial SPSR for kernel tasks (EL1h, IRQ enabled) */
#define PSR_SVC             PSR_EL1h
#define PSR_USR             PSR_EL0t
#define PSR_SYS             PSR_EL1h

#define TMF_CPL(n)          ((UINT)(n))
#define TMF_PPL(n)          ((UINT)(n) << 16)

/* Software interrupt numbers (logical — RV64 uses HVC/SVC separately) */
#define SWI_SVC             0
#define SWI_RETINT          1
#define SWI_DISPATCH        2
#define SWI_RETTEX          3
#define SWI_DEBUG           4
#define SWI_MONITOR         5

#define EIT_DEFAULT         0
#define EIT_FIQ             1

#endif
