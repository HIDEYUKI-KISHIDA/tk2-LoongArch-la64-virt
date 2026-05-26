/* tk_port_kernel.c — M5b t_kernel_main with per-module error codes on UART */

#include "kernel.h"
#include "timer.h"
#include "task.h"
#include <tm/tmonitor.h>

extern void uart_puts(const char *s);
extern void uart_put_er(ER ercd);

IMPORT ER cpu_initialize(void);
IMPORT ER tkdev_initialize(void);
IMPORT ER task_initialize(void);
IMPORT ER mutex_initialize(void);
IMPORT ER subsystem_initialize(void);
IMPORT ER resource_group_initialize(void);
IMPORT ER timer_initialize(void);

static void init_module_dbg(ER (*initfunc)(void), const char *name)
{
    ER ercd;

    uart_puts("[M5b] init ");
    uart_puts(name);
    uart_puts("...");
    ercd = (*initfunc)();
    if (ercd < E_OK) {
        uart_puts(" FAIL");
        uart_put_er(ercd);
        uart_puts("\r\n");
        tm_putstring((UB *)name);
        tm_putstring((UB *)" : module initialize error\n");
        tm_monitor();
    }
    uart_puts(" OK\r\n");
}

static void init_task_startup(T_CTSK *ctsk)
{
    ER ercd;

    ercd = _tk_cre_tsk(ctsk);
    if (ercd < E_OK) {
        uart_puts("[M5b] _tk_cre_tsk FAIL");
        uart_put_er(ercd);
        uart_puts("\r\n");
        tm_monitor();
    }

    ercd = _tk_sta_tsk((ID)ercd, 0);
    if (ercd < E_OK) {
        uart_puts("[M5b] _tk_sta_tsk FAIL");
        uart_put_er(ercd);
        uart_puts("\r\n");
        tm_monitor();
    }
}

void m5b_t_kernel_main(T_CTSK *inittask)
{
    DISABLE_INTERRUPT;

    init_module_dbg(cpu_initialize, "cpu");
    init_module_dbg(tkdev_initialize, "tkdev");
    init_module_dbg(task_initialize, "task");
#ifdef NUM_MTXID
    init_module_dbg(mutex_initialize, "mutex");
#endif
    init_module_dbg(subsystem_initialize, "subsystem");
    init_module_dbg(resource_group_initialize, "resource_group");
    init_module_dbg(timer_initialize, "timer");

    uart_puts("[M5b] init_task startup\r\n");
    init_task_startup(inittask);

    uart_puts("[M5b] force_dispatch\r\n");
    force_dispatch();
}
