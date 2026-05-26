#include <stdint.h>

#define CLINT_FREQ      10000000UL

extern uint64_t tk2_clint_read_mtime(void);
extern uint64_t tk2_clint_read_mtimecmp(unsigned hart);
extern void tk2_clint_write_mtimecmp(unsigned hart, uint64_t v);

static uint64_t g_timer_interval;

void sys_timer_initialize(int hartid)
{
    unsigned h = (unsigned)hartid;
    uint64_t now = tk2_clint_read_mtime();

    tk2_clint_write_mtimecmp(h, now + g_timer_interval);
    {
        unsigned long mie = 128UL;
        asm volatile("csrs mie, %0" :: "r"(mie));
    }
}

void sys_timer_interrupt_handler(int hartid)
{
    unsigned h = (unsigned)hartid;
    uint64_t now = tk2_clint_read_mtime();
    uint64_t cmp = tk2_clint_read_mtimecmp(h);
    uint64_t next = cmp + g_timer_interval;

    if ((long)(next - now) <= 0) {
        next = now + g_timer_interval;
    }
    tk2_clint_write_mtimecmp(h, next);
}

void timer_init(unsigned int hz)
{
    if (hz == 0) {
        hz = 100;
    }
    g_timer_interval = CLINT_FREQ / (uint64_t)hz;
    sys_timer_initialize(0);
}

void timer_on_irq(void)
{
    sys_timer_interrupt_handler(0);
}
