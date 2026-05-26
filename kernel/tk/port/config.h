/*
 * config.h (M5b virt AArch64) — minimal T-Kernel objects for M5 test
 * Included before upstream tk/src/config.h via -I kernel/tk/port
 */

#ifndef _CONFIG_
#define _CONFIG_

#include "knlconfig.h"

/* Task */
#define MIN_TSKID       (1)
#define MAX_TSKID       (max_tskid)
#define NUM_TSKID       (MAX_TSKID)
#define CHK_TSKID(id)   ((MIN_TSKID) <= (id) && (id) <= (MAX_TSKID))
#define INDEX_TSK(id)   ((id) - (MIN_TSKID))
#define ID_TSK(index)   ((index) + (MIN_TSKID))

/* Mutex only (M5 AB test) — omit NUM_SEMID, NUM_FLGID, ... */
#define MIN_MTXID       (1)
#define MAX_MTXID       (max_mtxid)
#define NUM_MTXID       (MAX_MTXID)
#define CHK_MTXID(id)   ((MIN_MTXID) <= (id) && (id) <= (MAX_MTXID))
#define INDEX_MTX(id)   ((id) - (MIN_MTXID))
#define ID_MTX(index)   ((index) + (MIN_MTXID))

/* Subsystem / resource group (required by subsystem.c) */
#define MIN_SSYID       (1)
#define MAX_SSYID       (max_ssyid)
#define NUM_SSYID       (MAX_SSYID)
#define CHK_SSYID(id)   ((MIN_SSYID) <= (id) && (id) <= (MAX_SSYID))
#define INDEX_SSY(id)   ((id) - (MIN_SSYID))
#define ID_SSY(index)   ((index) + (MIN_SSYID))

#define MIN_RESID       (1)
#define MAX_RESID       (max_resid)
#define NUM_RESID       (MAX_RESID)
#define CHK_RESID(id)   ((MIN_RESID) <= (id) && (id) <= (MAX_RESID))
#define INDEX_RES(id)   ((id) - (MIN_RESID))
#define ID_RES(index)   ((index) + (MIN_RESID))

#define MIN_PRI         (1)
#define MAX_PRI         (140)
#define NUM_PRI         (140)
#define CHK_PRI(pri)    ((MIN_PRI) <= (pri) && (pri) <= (MAX_PRI))

#define MIN_SSYPRI      (1)
#define MAX_SSYPRI      (max_ssypri)
#define NUM_SSYPRI      (MAX_SSYPRI)
#define CHK_SSYPRI(pri) ((MIN_SSYPRI) <= (pri) && (pri) <= (MAX_SSYPRI))

#define CHK_NOSPT       (1)
#define CHK_RSATR       (1)
#define CHK_PAR         (1)
#define CHK_ID          (1)
#define CHK_OACV        (1)
#define CHK_CTX         (1)
#define CHK_CTX1        (1)
#define CHK_CTX2        (1)
#define CHK_SELF        (1)
#define CHK_NOCOP       (1)

#define USE_DBGSPT      (0)
#define USE_OBJECT_NAME (1)
#define OBJECT_NAME_LENGTH (8)
#define USE_KERNEL_MESSAGE (1)

#endif /* _CONFIG_ */
