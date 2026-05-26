#ifndef M5B_TRACE_H
#define M5B_TRACE_H

#include <stdint.h>

/* Legacy single-char trace (optional; keep off in normal builds) */
#define M5B_TRACE_LEN  64

#define M5B_LOG_ENTRIES  48

/* Event kinds (ASCII tag in log dump) */
#define M5B_LOG_SAVE   'S'  /* dispatch_entry saved ctxtsk */
#define M5B_LOG_REST   'R'  /* dispatch restore (from asm via m5b_log_restore) */
#define M5B_LOG_FD     'F'  /* force_dispatch target */
#define M5B_LOG_EXT    'X'  /* tk_ext_tsk */
#define M5B_LOG_DORM   'D'  /* make_dormant / setup_context */
#define M5B_LOG_BAD    'B'  /* bad ssp / frame */

typedef struct {
    uint32_t seq;
    uint8_t  kind;
    uint8_t  tskid;
    uint8_t  state;
    uint8_t  pad;
    uint64_t ssp;
    uint64_t lr88;     /* [ssp+88] x30 slot */
    uint64_t elr96;    /* [ssp+96] */
    uint64_t spsr104;  /* [ssp+104] */
    uint64_t spk112;   /* [ssp+112] SP_KSP */
} m5b_log_ent_t;

void m5b_trace(char tag);
void m5b_trace_dump(void);

/* Ring log: no UART until panic dump */
void m5b_log_event(uint8_t kind, const void *tcb, const void *ssp);
void m5b_log_restore(uint64_t ssp, uint64_t x30, uint64_t elr96, uint64_t spsr);

/* Patch x30 slot from ELR copy when corrupted (call before dispatch) */
void m5b_frame_fixup_lr(void *ssp);

extern volatile uint64_t m5b_last_ssp;
extern volatile uint64_t m5b_last_x30;
extern volatile uint64_t m5b_last_spsr;
extern volatile uint64_t m5b_last_elr_slot;

/* 1 while in tk_ext_tsk path: block END_CRITICAL enaint/dispatch until force_dispatch */
extern volatile int m5b_ext_tsk_hold_int;

/* tmp_dispatch_stack in cpu_support_tk.S — use during tk_ext_tsk so make_dormant
 * does not clobber the outgoing task's stack (return chain / locals). */
extern char tmp_dispatch_stack[];
void m5b_use_tmp_stack(void);

#endif
