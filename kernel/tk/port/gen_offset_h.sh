#!/bin/bash
# Usage: CROSS=aarch64-linux-gnu- gen_offset_h.sh build/offset_vals.o > offset.h
OBJ="$1"
NM="${CROSS}nm"

hex_of() {
    "$NM" "$OBJ" | awk -v s="off_export_$1" '$3 == s { print $1; exit }'
}

dec_of() {
    local hex
    hex=$(hex_of "$1")
    if [ -z "$hex" ]; then
        echo "gen_offset_h: missing off_export_$1" >&2
        exit 1
    fi
    echo $((16#$hex))
}

cat <<EOF
/* AUTO-GENERATED from offset_vals.o (${CROSS}gcc) - do not edit */
#ifndef _OFFSET_
#define _OFFSET_

#define _ALIGN_CPU(x) (((x)+7)&0xFFFFFFFFFFFFFFF8ULL)

#define TCB_tskid $(dec_of tskid)
#define TCB_tskatr $(dec_of tskatr)
#define TCB_state $(dec_of state)
#define TCB_reqdct $(dec_of reqdct)
#define TCB_tskctxb $(dec_of tskctxb)
#define CTXB_ssp 0
#define CTXB_uatb $(dec_of CTXB_uatb)
#define CTXB_lsid $(dec_of CTXB_lsid)
#define CTXB_svc_ssp $(dec_of CTXB_svc_ssp)
#define TA_FPU 0

#endif
EOF
