#include "tk2_la64_hw.h"

#define CSR_ESTAT_IS_MASK  0x00001FFFUL
#define ESTAT_ECODE(es)    (((es) >> 16) & 0x3F)

static void uart_put_hex64(unsigned long v)
{
    static const char hex[] = "0123456789abcdef";
    int shift;

    uart_puts("0x");
    for (shift = 60; shift >= 0; shift -= 4)
        uart_putc(hex[(v >> shift) & 0xFUL]);
}

static void kernel_panic_estat(const char *msg, unsigned long estat)
{
    uart_puts("\n[PANIC] ");
    uart_puts(msg);
    uart_puts(" estat=");
    uart_put_hex64(estat);
    uart_puts(" ecode=");
    uart_put_hex64((unsigned long)ESTAT_ECODE(estat));
    uart_puts("\n");
    for (;;)
        __asm__ volatile("idle 0");
}

void tk_handle_exception(TRAP_CTX_LA64 *ef)
{
    unsigned long estat = ef->estat;
    unsigned int  ecode = ESTAT_ECODE(estat);

    if (ecode == 0x0B) {
        uart_puts("SYSCALL (stub)\n");
        ef->era += 4;
        return;
    }

    if (estat & CSR_ESTAT_IS_MASK) {
        if (estat & (1UL << 11))
            timer_interrupt_handler();
        else
            kernel_panic_estat("unhandled IRQ", estat);
        return;
    }

    kernel_panic_estat("unhandled exception", estat);
}
