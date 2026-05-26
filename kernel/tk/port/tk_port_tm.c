/* tk_port_tm.c — T-Monitor stubs → PL011 UART */

#include <tm/tmonitor.h>

extern void uart_puts(const char *s);
extern void uart_putc(char c);

INT tm_putchar(INT c)
{
    uart_putc((char)(c & 0xff));
    return c;
}

INT tm_putstring(const UB *str)
{
    if (str != 0) {
        uart_puts((const char *)str);
    }
    return 0;
}

void tm_monitor(void)
{
    uart_puts("\r\n[tm_monitor] halted\r\n");
    for (;;) {
        asm volatile("idle 0");
    }
}

void tm_exit(INT mode)
{
    (void)mode;
    tm_monitor();
}
