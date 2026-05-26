#include <tk/typedef.h>

extern void virt_timer_on_irq_tk(void);

void irq_handler(void)
{
    virt_timer_on_irq_tk();
}
