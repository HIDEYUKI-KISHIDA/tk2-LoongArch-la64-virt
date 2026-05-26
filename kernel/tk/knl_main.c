extern void uart_puts(const char *s);

#if defined(M5B_TKERNEL) || defined(TK2_TKERNEL)
extern void tk_port_boot(void);

void knl_main(void)
{
    uart_puts("Hello LoongArch LA64 TK2\r\n");
    uart_puts("Running at PLV0 (DA)\r\n");
    tk_port_boot();
}
#else
extern void m5_run_tk_task(void);

void knl_main(void)
{
    uart_puts("Hello LoongArch LA64 TK2\r\n");
    m5_run_tk_task();
    for (;;) {
        __asm__ volatile("idle 0");
    }
}
#endif
