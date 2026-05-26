/* tk_port_boot.c — M5b: board_init → t_kernel_main (official TK2) */

#include "knlconfig.h"
#include <tk/tkernel.h>
#include <tk/typedef.h>
#include <tm/tmonitor.h>

extern void uart_puts(const char *s);
extern void m5b_t_kernel_main(T_CTSK *inittask);
extern ER init_Imalloc(void);

extern void irq_disable(void);

static void irq_mask_all(void)
{
    irq_disable();
}

static void m5b_init_task(void);

static const T_CTSK c_m5b_init = {
    .tskatr = TA_HLNG | TA_RNG0,
    .task = (FP)m5b_init_task,
    .itskpri = 12,
    .stksz = 8192,
    .sstksz = 0,
    .stkptr = 0,
    .uatb = 0,
    .lsid = 0,
    .resid = 0,
};

static void m5b_init_task(void)
{
    extern void m5_run_tk_task(void);
    uart_puts("[M5b] init_task running\r\n");
    m5_run_tk_task();
    uart_puts("[M5b] init_task done\r\n");
    tk_ext_tsk();
}

void tk_port_boot(void)
{
    uart_puts("M5b tk_port_boot\r\n");
    /* board_init は reset.S で済み。二重 MMU/GIC 初期化を避ける */
    irq_mask_all();
    (void)init_Imalloc();
    uart_puts("[M5b] calling m5b_t_kernel_main\r\n");
    m5b_t_kernel_main((T_CTSK *)&c_m5b_init);
    uart_puts("[M5b] m5b_t_kernel_main returned?\r\n");
    for (;;) {
        __asm__ volatile("idle 0");
    }
}

void DispProgress(W n)
{
    (void)n;
}
