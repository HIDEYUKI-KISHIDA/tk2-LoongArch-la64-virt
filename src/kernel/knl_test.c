#include "tk2_la64_hw.h"

#define TEST_MIN_TICKS  5u
#define TEST_SPIN       8192u

static int test_timer_tick(void)
{
    UD start = g_tick_count;
    UD deadline = start + TEST_MIN_TICKS;
    unsigned n;

    uart_puts("  [test] 100Hz tick (poll)...\n");
    irq_disable();
    for (n = 0; g_tick_count < deadline && n < 500000u; ++n) {
        timer_poll();
        for (volatile UD d = 0; d < TEST_SPIN; ++d)
            __asm__ volatile("nop");
    }
    irq_enable();
    if (g_tick_count < start + TEST_MIN_TICKS)
        return -1;
    return 0;
}

static int test_tick_monotonic(void)
{
    UD a = g_tick_count;
    unsigned n;

    irq_disable();
    for (n = 0; n < 20000u; ++n) {
        timer_poll();
        if (g_tick_count > a) {
            irq_enable();
            return 0;
        }
        for (volatile UD d = 0; d < TEST_SPIN; ++d)
            __asm__ volatile("nop");
    }
    irq_enable();
    return -1;
}

int knl_run_tests(void)
{
    int fail = 0;

    uart_puts("TK2-LA64: Phase4 test suite\n");

    if (test_timer_tick() != 0) {
        uart_puts("  FAIL: timer tick\n");
        fail = 1;
    } else {
        uart_puts("  PASS: timer tick\n");
    }

    if (test_tick_monotonic() != 0) {
        uart_puts("  FAIL: tick monotonic\n");
        fail = 1;
    } else {
        uart_puts("  PASS: tick monotonic\n");
    }

    return fail;
}
