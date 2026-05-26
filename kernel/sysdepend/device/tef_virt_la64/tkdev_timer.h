#ifndef _TKDEV_TIMER_
#define _TKDEV_TIMER_

#include <tk/typedef.h>

#define MIN_TIMER_PERIOD    1000
#define MAX_TIMER_PERIOD    1000000

IMPORT void virt_timer_init_for_tk(void);
IMPORT void virt_timer_on_irq_tk(void);
void irq_enable(void);

Inline void init_hw_timer(void) { virt_timer_init_for_tk(); }
Inline void start_hw_timer(void)
{
    virt_timer_init_for_tk();
    irq_enable();
}
Inline void terminate_hw_timer(void) { }
Inline void clear_hw_timer_interrupt(void) { }
Inline void end_of_hw_timer_interrupt(void) { }

Inline UINT get_hw_timer_usec(void) { return 0; }

#endif
