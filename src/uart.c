#include "tk2_la64_hw.h"

#define UART_THR  (*(volatile unsigned char *)(UART_BASE + 0x00))
#define UART_LSR  (*(volatile unsigned char *)(UART_BASE + 0x05))
#define UART_LSR_THRE  0x20

void uart_putc(char c)
{
    while (!(UART_LSR & UART_LSR_THRE))
        ;
    UART_THR = (unsigned char)c;
}

void uart_puts(const char *s)
{
    while (*s) {
        if (*s == '\n')
            uart_putc('\r');
        uart_putc(*s++);
    }
}
