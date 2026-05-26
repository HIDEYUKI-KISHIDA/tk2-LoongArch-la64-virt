#include <stdint.h>

#define CSR_TCFG   0x41
#define CSR_TICLR  0x44

static unsigned long timer_freq(void)
{
    unsigned int val;
    __asm__ volatile("cpucfg %0, %1" : "=r"(val) : "r"(4U));
    return (unsigned long)val ? (unsigned long)val : 10000000UL;
}

static unsigned long g_interval;

void sys_timer_initialize(int hartid)
{
    unsigned long freq = timer_freq();
    unsigned long interval = freq / 100UL;
    unsigned long tcfg = (1UL << 0) | (1UL << 1) | (interval << 2);
    (void)hartid;
    __asm__ volatile("csrwr %0, %1" :: "r"(tcfg), "i"(CSR_TCFG));
}

void sys_timer_interrupt_handler(int hartid)
{
    (void)hartid;
    __asm__ volatile("csrwr %0, %1" :: "r"(1UL), "i"(CSR_TICLR));
}

void timer_init(unsigned int hz)
{
    if (hz == 0) {
        hz = 100;
    }
    g_interval = timer_freq() / (unsigned long)hz;
    sys_timer_initialize(0);
}

void timer_on_irq(void)
{
    sys_timer_interrupt_handler(0);
}
