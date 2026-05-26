#include "tk2_la64_hw.h"

#define TASK_STACK_WORDS  512u
#define CRMD_PLV0_DA      0x08UL
/* 16 round-trips A→B→A (32 switches); avoids endless ABAB + IRQ during switch */
#define SCHED_SWITCH_MAX  32u

extern void switch_context(TASK_CTX_LA64 *prev, TASK_CTX_LA64 *next);

static UD task_a_stack[TASK_STACK_WORDS];
static UD task_b_stack[TASK_STACK_WORDS];

static TASK_CTX_LA64 ctx_idle;
static TASK_CTX_LA64 ctx_a;
static TASK_CTX_LA64 ctx_b;
static unsigned   g_sched_switches;

static UD task_stack_top(UD *stk, UD n_words)
{
    return (UD)&stk[n_words] & ~(UD)15;
}

static void task_ctx_init(TASK_CTX_LA64 *ctx, UD *stack, UD n_words,
                          void (*entry)(void))
{
    unsigned i;

    for (i = 0; i < 9; ++i)
        ctx->s[i] = 0;
    ctx->fp  = 0;
    ctx->sp  = task_stack_top(stack, n_words);
    ctx->ra  = (UD)(VP)entry;
    ctx->era = (UD)(VP)entry;
    /* IE off in saved CRMD; irq_enable() after switch (Rev.6.1 §3.11) */
    ctx->crmd = CRMD_PLV0_DA;
}

static void task_a_entry(void) __attribute__((noreturn));
static void task_b_entry(void) __attribute__((noreturn));

static void task_a_entry(void)
{
    for (;;) {
        uart_putc('A');
        irq_disable();
        switch_context(&ctx_a, &ctx_b);
        irq_enable();
        if (++g_sched_switches >= SCHED_SWITCH_MAX) {
            uart_puts("\nTK2-LA64: Phase4 scheduler OK\n");
            for (;;)
                __asm__ volatile("idle 0");
        }
    }
}

static void task_b_entry(void)
{
    for (;;) {
        uart_putc('B');
        irq_disable();
        switch_context(&ctx_b, &ctx_a);
        irq_enable();
    }
}

void knl_sched_start(void)
{
    task_ctx_init(&ctx_a, task_a_stack, TASK_STACK_WORDS, task_a_entry);
    task_ctx_init(&ctx_b, task_b_stack, TASK_STACK_WORDS, task_b_entry);
    irq_disable();
    switch_context(&ctx_idle, &ctx_a);
    irq_enable();
}
