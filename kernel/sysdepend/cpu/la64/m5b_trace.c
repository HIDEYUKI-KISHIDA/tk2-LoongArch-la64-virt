#include "m5b_trace.h"

#include "kernel.h"
#include "task.h"
#include "cpu_attr.h"

#include <stdint.h>

extern void uart_puts(const char *s);
extern void uart_putc(char c);

IMPORT TCB *schedtsk;

volatile char m5b_trace_ring[M5B_TRACE_LEN];
volatile unsigned int m5b_trace_idx;

volatile uint64_t m5b_last_ssp;
volatile uint64_t m5b_last_x30;
volatile uint64_t m5b_last_spsr;
volatile uint64_t m5b_last_elr_slot;

volatile int m5b_ext_tsk_hold_int;

void m5b_use_tmp_stack(void)
{
    uintptr_t top = (uintptr_t)tmp_dispatch_stack + 4096u;

    asm volatile("move $sp, %0" :: "r"(top) : "memory");
}

static volatile m5b_log_ent_t m5b_log_ring[M5B_LOG_ENTRIES];
static volatile uint32_t m5b_log_seq;
static volatile unsigned int m5b_log_head;

void m5b_trace(char tag)
{
    unsigned int i = m5b_trace_idx;

    if (i >= M5B_TRACE_LEN) {
        i = M5B_TRACE_LEN - 1;
    }
    m5b_trace_ring[i] = tag;
    m5b_trace_idx = i + 1;
}

static void log_push(uint8_t kind, const TCB *tcb, uintptr_t ssp)
{
    unsigned int i = m5b_log_head % M5B_LOG_ENTRIES;
    m5b_log_ent_t *e = (m5b_log_ent_t *)&m5b_log_ring[i];

    e->seq = ++m5b_log_seq;
    e->kind = kind;
    e->tskid = (tcb != NULL) ? (uint8_t)tcb->tskid : 0;
    e->state = (tcb != NULL) ? (uint8_t)tcb->state : 0;
    e->ssp = (uint64_t)ssp;
    if (ssp != 0) {
        e->lr88 = *(uint64_t *)(ssp + TK2_CTX_X30);
        e->elr96 = *(uint64_t *)(ssp + TK2_CTX_ELR);
        e->spsr104 = *(uint64_t *)(ssp + TK2_CTX_SPSR);
        e->spk112 = *(uint64_t *)(ssp + TK2_CTX_SP_KSP);
    } else {
        e->lr88 = e->elr96 = e->spsr104 = e->spk112 = 0;
    }
    m5b_log_head = i + 1;
}

void m5b_log_event(uint8_t kind, const void *tcb, const void *ssp)
{
    log_push(kind, (const TCB *)tcb, (uintptr_t)ssp);
}

void m5b_log_restore(uint64_t ssp, uint64_t x30, uint64_t elr96, uint64_t spsr)
{
    unsigned int i = m5b_log_head % M5B_LOG_ENTRIES;
    m5b_log_ent_t *e = (m5b_log_ent_t *)&m5b_log_ring[i];

    e->seq = ++m5b_log_seq;
    e->kind = M5B_LOG_REST;
    e->tskid = (schedtsk != NULL) ? (uint8_t)schedtsk->tskid : 0;
    e->state = (schedtsk != NULL) ? (uint8_t)schedtsk->state : 0;
    e->ssp = ssp;
    e->lr88 = x30;
    e->elr96 = elr96;
    e->spsr104 = spsr;
    e->spk112 = (ssp != 0) ? *(uint64_t *)(uintptr_t)(ssp + TK2_CTX_SP_KSP) : 0;
    m5b_log_head = i + 1;
}

void m5b_frame_fixup_lr(void *ssp)
{
    uintptr_t base = (uintptr_t)ssp;
    uintptr_t lr;
    uintptr_t elr;

    if (ssp == NULL) {
        return;
    }
    lr = *(uintptr_t *)(base + TK2_CTX_X30);
    elr = *(uintptr_t *)(base + TK2_CTX_ELR);
    if (lr < 0x1000UL && elr >= 0x1000UL) {
        *(uintptr_t *)(base + TK2_CTX_X30) = elr;
        m5b_log_event(M5B_LOG_BAD, schedtsk, ssp);
    }
}

static void put_hex64(uint64_t v)
{
    static const char hex[] = "0123456789abcdef";
    int n;

    uart_puts("0x");
    for (n = 60; n >= 0; n -= 4) {
        uart_putc(hex[(v >> n) & 0xf]);
    }
}

static void log_dump_one(const m5b_log_ent_t *e)
{
    uart_putc((char)e->kind);
    uart_puts(" #");
    put_hex64((uint64_t)e->seq);
    uart_puts(" id=");
    put_hex64((uint64_t)e->tskid);
    uart_puts(" ssp=");
    put_hex64(e->ssp);
    uart_puts(" lr88=");
    put_hex64(e->lr88);
    uart_puts(" elr96=");
    put_hex64(e->elr96);
    uart_puts(" spsr=");
    put_hex64(e->spsr104);
    uart_puts(" ksp=");
    put_hex64(e->spk112);
    uart_puts("\r\n");
}

static void m5b_log_dump(void)
{
    unsigned int total = m5b_log_head;
    unsigned int start;
    unsigned int i;

    if (total > M5B_LOG_ENTRIES) {
        start = total - M5B_LOG_ENTRIES;
    } else {
        start = 0;
    }

    uart_puts("--- dispatch log (ring) ---\r\n");

    for (i = start; i < total; i++) {
        log_dump_one((const m5b_log_ent_t *)&m5b_log_ring[i % M5B_LOG_ENTRIES]);
    }
}

void m5b_trace_dump(void)
{
    unsigned int i, n = m5b_trace_idx;

    m5b_log_dump();

    if (n > M5B_TRACE_LEN) {
        n = M5B_TRACE_LEN;
    }

    uart_puts("TRACE chars: ");
    for (i = 0; i < n; i++) {
        uart_putc(m5b_trace_ring[i]);
    }
    uart_puts("\r\n");

    uart_puts("last restore ssp=");
    put_hex64(m5b_last_ssp);
    uart_puts(" x30=");
    put_hex64(m5b_last_x30);
    uart_puts(" elr96=");
    put_hex64(m5b_last_elr_slot);
    uart_puts(" spsr=");
    put_hex64(m5b_last_spsr);
    uart_puts("\r\n");

    if (schedtsk != NULL && schedtsk->tskctxb.ssp != NULL) {
        uintptr_t ssp = (uintptr_t)schedtsk->tskctxb.ssp;

        uart_puts("sched tskid=");
        put_hex64((uint64_t)schedtsk->tskid);
        uart_puts(" lr88=");
        put_hex64(*(uint64_t *)(ssp + TK2_CTX_X30));
        uart_puts(" elr96=");
        put_hex64(*(uint64_t *)(ssp + TK2_CTX_ELR));
        uart_puts(" ksp=");
        put_hex64(*(uint64_t *)(ssp + TK2_CTX_SP_KSP));
        uart_puts("\r\n");
    }
}
