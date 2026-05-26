#include "tk2_la64_hw.h"

extern int knl_run_tests(void);
extern void knl_sched_start(void);

void knl_main(void)
{
    uart_puts("[TK2] LoongArch LA64 Boot OK\n");
    uart_puts("TK2-LA64: Phase1 UART OK\n");

    timer_init();
    irq_enable();

    uart_puts("TK2-LA64: Phase2 timer + IRQ enabled\n");

    if (knl_run_tests() != 0) {
        uart_puts("TK2-LA64: tests FAILED — halt\n");
        for (;;)
            __asm__ volatile("idle 0");
    }

    uart_puts("TK2-LA64: tests PASS — scheduler A/B\n");
    knl_sched_start();

    for (;;)
        __asm__ volatile("idle 0");
}
