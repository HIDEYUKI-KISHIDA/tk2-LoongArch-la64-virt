#ifndef TK2_LA64_HW_H
#define TK2_LA64_HW_H

#include "tk2_la64_types.h"

/* QEMU virt NS16550A PA 0x1fe001e0; uncached direct-map VA (XKPRANGE / DMW0) */
#define UART_BASE        0x800000001fe001e0UL
#define TICK_HZ          100UL

void uart_putc(char c);
void uart_puts(const char *s);

void exception_vector_init(void);
void irq_enable(void);
void irq_disable(void);

void timer_init(void);
void timer_interrupt_handler(void);
void timer_poll(void);

void tk_handle_exception(TRAP_CTX_LA64 *ef);

void switch_context(TASK_CTX_LA64 *prev, TASK_CTX_LA64 *next);

void knl_main(void);
void knl_sched_start(void);
int knl_run_tests(void);

extern volatile UD g_tick_count;

#endif
