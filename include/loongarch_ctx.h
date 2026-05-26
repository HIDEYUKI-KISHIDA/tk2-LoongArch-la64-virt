#ifndef LOONGARCH_CTX_H
#define LOONGARCH_CTX_H

typedef struct {
    unsigned long r[31];
    unsigned long era;
    unsigned long crmd;
    unsigned long prmd;
    unsigned long estat;
    unsigned long badv;
} TK_EXFRAME_LOONGARCH;

typedef struct {
    unsigned long ra;
    unsigned long sp;
    unsigned long fp;
    unsigned long s[9];
    unsigned long era;
    unsigned long crmd;
} TK_TASKCTX_LOONGARCH;

_Static_assert(sizeof(TK_EXFRAME_LOONGARCH) == 288,
               "TK_EXFRAME_LOONGARCH must be 288 bytes");
_Static_assert(sizeof(TK_TASKCTX_LOONGARCH) == 112,
               "TK_TASKCTX_LOONGARCH must be 112 bytes");

#endif
