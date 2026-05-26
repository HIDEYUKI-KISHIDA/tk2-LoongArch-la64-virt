/*
 * rominfo_depend.h (sys/tef_virt_la64) — SYSCONF tags for M5b static _tk_get_cfn
 */

#ifndef __SYS_ROMINFO_DEPEND_H__
#define __SYS_ROMINFO_DEPEND_H__

#include <basic.h>

typedef struct {
    FP   kernel;
    UB  *sysconf;
    UB  *devconf;
    void *userarea;
    FP   userinit;
    FP   resetinit;
    VW   rsv[10];
    UW   rd_type;
    UW   rd_blksz;
    UW   rd_saddr;
    UW   rd_eaddr;
    VW   rsv2[12];
} RomInfo;

#define SCTAG_TSYSNAME      ((UB *)"TSysName")
#define SCTAG_MAKER         ((UB *)"Maker")
#define SCTAG_PRODUCTID     ((UB *)"ProductID")
#define SCTAG_SPECVER       ((UB *)"SpecVer")
#define SCTAG_PRODUCTVER    ((UB *)"ProductVer")
#define SCTAG_PRODUCTNO     ((UB *)"ProductNo")

#define SCTAG_TMAXTSKID     ((UB *)"TMaxTskId")
#define SCTAG_TMAXSEMID     ((UB *)"TMaxSemId")
#define SCTAG_TMAXFLGID     ((UB *)"TMaxFlgId")
#define SCTAG_TMAXMBXID     ((UB *)"TMaxMbxId")
#define SCTAG_TMAXMTXID     ((UB *)"TMaxMtxId")
#define SCTAG_TMAXMBFID     ((UB *)"TMaxMbfId")
#define SCTAG_TMAXPORID     ((UB *)"TMaxPorId")
#define SCTAG_TMAXMPFID     ((UB *)"TMaxMpfId")
#define SCTAG_TMAXMPLID     ((UB *)"TMaxMplId")
#define SCTAG_TMAXCYCID     ((UB *)"TMaxCycId")
#define SCTAG_TMAXALMID     ((UB *)"TMaxAlmId")
#define SCTAG_TMAXRESID     ((UB *)"TMaxResId")
#define SCTAG_TMAXSSYID     ((UB *)"TMaxSsyId")
#define SCTAG_TMAXSSYPRI    ((UB *)"TMaxSsyPri")

#define SCTAG_TSYSSTKSZ     ((UB *)"TSysStkSz")
#define SCTAG_TSVCLIMIT     ((UB *)"TSVCLimit")
#define SCTAG_TTIMPERIOD    ((UB *)"TTimPeriod")

#endif
