#include <stdint.h>

extern void uart_puts(const char *s);

void kernel_sync_exception_handler(unsigned long trap_frame)
{
    (void)trap_frame;
    uart_puts("\r\n[LA64] sync exception — halt\r\n");
}
