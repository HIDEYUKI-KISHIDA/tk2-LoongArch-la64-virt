#include "board_config.h"

extern void uart_puts(const char *s);
extern void timer_init(unsigned int hz);
extern void exception_vector_init(void);
extern void irq_enable(void);

void board_init(void)
{
    uart_puts(BOARD_INIT_TAG "\r\n");
    uart_puts("[board_init] UART OK\r\n");
    timer_init(100);
    uart_puts("[board_init] core timer OK\r\n");
    irq_enable();
    uart_puts("[board_init] IRQ enabled\r\n");
}
