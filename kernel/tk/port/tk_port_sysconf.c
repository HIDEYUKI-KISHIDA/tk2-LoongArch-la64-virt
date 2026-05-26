/* tk_port_sysconf.c — M5b static SYSCONF for bare-metal QEMU (no config DB) */

#include "knlconfig.h"
#include <basic.h>
#include <sys/rominfo.h>
#include <libstr.h>

static int cfn_tag_eq(CONST UB *name, const char *lit)
{
    while (*lit != '\0') {
        if (*name != (UB)*lit) {
            return 0;
        }
        name++;
        lit++;
    }
    return *name == (UB)'\0';
}

INT _tk_get_cfn(CONST UB *name, INT *val, INT max)
{
    if (name == 0 || val == 0 || max < 1) {
        return 0;
    }

    if (cfn_tag_eq(name, "TMaxTskId")) {
        val[0] = M5B_MAX_TSKID;
        return 1;
    }
    if (cfn_tag_eq(name, "TSysStkSz")) {
        val[0] = M5B_SYSSTKSZ;
        return 1;
    }
    if (cfn_tag_eq(name, "TSVCLimit")) {
        val[0] = 1;
        return 1;
    }
    if (cfn_tag_eq(name, "TTimPeriod")) {
        val[0] = M5B_TTIMPERIOD / 1000;
        if (max >= 2) {
            val[1] = (M5B_TTIMPERIOD % 1000) * 1000;
        }
        return (max >= 2) ? 2 : 1;
    }
    if (cfn_tag_eq(name, "TMaxMtxId")) {
        val[0] = M5B_MAX_MTXID;
        return 1;
    }
    if (cfn_tag_eq(name, "TMaxSemId")) {
        val[0] = 0;
        return 1;
    }
    if (cfn_tag_eq(name, "TMaxFlgId")) {
        val[0] = 0;
        return 1;
    }
    if (cfn_tag_eq(name, "TMaxMbxId")) {
        val[0] = 0;
        return 1;
    }
    if (cfn_tag_eq(name, "TMaxMbfId")) {
        val[0] = 0;
        return 1;
    }
    if (cfn_tag_eq(name, "TMaxPorId")) {
        val[0] = 0;
        return 1;
    }
    if (cfn_tag_eq(name, "TMaxMplId")) {
        val[0] = 0;
        return 1;
    }
    if (cfn_tag_eq(name, "TMaxMpfId")) {
        val[0] = 0;
        return 1;
    }
    if (cfn_tag_eq(name, "TMaxCycId")) {
        val[0] = 0;
        return 1;
    }
    if (cfn_tag_eq(name, "TMaxAlmId")) {
        val[0] = 0;
        return 1;
    }
    if (cfn_tag_eq(name, "TMaxSsyId")) {
        val[0] = 1;
        return 1;
    }
    if (cfn_tag_eq(name, "TMaxSsyPri")) {
        val[0] = 1;
        return 1;
    }
    if (cfn_tag_eq(name, "TMaxResId")) {
        val[0] = 1;
        return 1;
    }

    return 0;
}
