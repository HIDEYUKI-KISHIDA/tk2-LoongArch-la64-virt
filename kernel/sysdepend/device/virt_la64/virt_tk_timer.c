#include <tk/typedef.h>
#include "m5b_trace.h"

extern void timer_init(unsigned int hz);
extern void timer_handler(void);
IMPORT RELTIM_U TIMER_PERIOD;

void virt_timer_init_for_tk(void)
{
    timer_init(100);
}

void virt_timer_on_irq_tk(void)
{
    if (TIMER_PERIOD == 0) {
        return;
    }
    if (m5b_ext_tsk_hold_int) {
        return;
    }
    timer_handler();
}
