#ifndef _CPU_INSN_
#define _CPU_INSN_

#include <tk/typedef.h>
#include <sys/sysinfo.h>

#define LOONGARCH_CSR_CRMD  0x00
#define CRMD_IE_BIT         (1UL << 2)

#define VECNO_DEFAULT   EIT_DEFAULT

typedef struct monhdr {
    FP default_hdr;
} MONHDR;

IMPORT MONHDR SaveMonHdr;

void irq_enable(void);

Inline void define_inthdr(INT vecno, FP inthdr)
{
    (void)vecno;
    (void)inthdr;
}

Inline BOOL isTaskIndependent(void)
{
    return (SCInfo.taskindp > 0) ? TRUE : FALSE;
}

Inline void EnterTaskIndependent(void)
{
    SCInfo.taskindp++;
}

Inline void LeaveTaskIndependent(void)
{
    SCInfo.taskindp--;
}

Inline UINT disint(void)
{
    UINT crmd_saved;
    UINT crmd_new;

    asm volatile("csrrd %0, %1" : "=r"(crmd_saved) : "i"(LOONGARCH_CSR_CRMD));
    crmd_new = crmd_saved & ~CRMD_IE_BIT;
    asm volatile("csrwr %0, %1" :: "r"(crmd_new), "i"(LOONGARCH_CSR_CRMD) : "memory");
    return crmd_saved;
}

Inline void enaint(UINT crmd_saved)
{
    if (crmd_saved == 0) {
        irq_enable();
    } else {
        asm volatile("csrwr %0, %1" :: "r"(crmd_saved), "i"(LOONGARCH_CSR_CRMD)
                     : "memory");
    }
}

Inline BOOL isDI(UINT crmd)
{
    return ((crmd & CRMD_IE_BIT) == 0) ? TRUE : FALSE;
}

#endif
