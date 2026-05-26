#ifndef __SYS_SYSINFO_DEPEND_H__
#define __SYS_SYSINFO_DEPEND_H__

#include <basic.h>

typedef struct {
    INT taskindp;
    UINT taskmode;
} SCInfoType;

typedef struct {
    SCInfoType scinfo;
} SysCommonArea;

#define SCArea  ((SysCommonArea *)&g_sc_area)
#define SCInfo  (SCArea->scinfo)
#define TASKMODE (SCInfo.taskmode)
#define TASKINDP (SCInfo.taskindp)

extern SysCommonArea g_sc_area;

#endif
