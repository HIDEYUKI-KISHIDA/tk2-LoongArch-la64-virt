/* tk2_la64_types.h — LP64 per TK2-LA64-SPEC Rev.6.1 §2A */
#ifndef TK2_LA64_TYPES_H
#define TK2_LA64_TYPES_H

typedef int             W;
typedef unsigned int    UW;
typedef long            D;
typedef unsigned long   UD;
typedef int             INT;
typedef unsigned int    UINT;
typedef long            REGINT;
typedef unsigned long   REGUINT;
typedef UD              SIZE;
typedef UD              ADDR;
typedef void           *VP;
typedef const void     *CVP;

#define TK_CAST_PTR_TO_REGUINT(p) \
    ((REGUINT)((VP)(p) - (VP)0))

#include "loongarch_ctx.h"

typedef TK_EXFRAME_LOONGARCH TRAP_CTX_LA64;
typedef TK_TASKCTX_LOONGARCH TASK_CTX_LA64;

#endif
