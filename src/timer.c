#include "tk2_la64_hw.h"

#define CSR_TCFG    0x41
#define CSR_TICLR   0x44
#define CSR_ESTAT   0x05
#define TCFG_EN     (1UL << 0)
#define TCFG_LOOP   (1UL << 1)

volatile UD g_tick_count;

static unsigned long timer_freq(void)
{
    unsigned int val;

    __asm__ volatile("cpucfg %0, %1" : "=r"(val) : "r"(4U));
    return (unsigned long)val;
}

void timer_init(void)
{
    unsigned long freq = timer_freq();

    if (freq == 0UL)
        freq = 10000000UL;
    {
    unsigned long interval = freq / TICK_HZ;
    unsigned long tcfg     = TCFG_EN | TCFG_LOOP | (interval << 2);

    __asm__ volatile("csrwr %0, %1" :: "r"(tcfg), "i"(CSR_TCFG));
    }
}

void timer_interrupt_handler(void)
{
    __asm__ volatile("csrwr %0, %1" :: "r"(1UL), "i"(CSR_TICLR));
    g_tick_count++;
    if ((g_tick_count % TICK_HZ) == 0UL)
        uart_puts(".\n");
}

void timer_poll(void)
{
    unsigned long estat;

    __asm__ volatile("csrrd %0, %1" : "=r"(estat) : "i"(CSR_ESTAT));
    if (estat & (1UL << 11))
        timer_interrupt_handler();
}
