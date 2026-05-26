/* tk_port_diag.c — M5b UART helpers for init failures */

#include <tk/typedef.h>

extern void uart_puts(const char *s);
extern void uart_putc(char c);

void uart_put_hex32(UINT v)
{
    static const char hex[] = "0123456789abcdef";
    char buf[11];
    int i;

    buf[0] = '0';
    buf[1] = 'x';
    for (i = 0; i < 8; i++) {
        buf[2 + i] = hex[(v >> (28 - i * 4)) & 0xfU];
    }
    buf[10] = '\0';
    uart_puts(buf);
}

void uart_put_er(ER ercd)
{
    uart_puts(" ercd=");
    uart_put_hex32((UINT)ercd);
}
