/*

 * Dispatch hooks for M5b RV64 port.

 */



#include <stddef.h>

#include <stdint.h>



#include "kernel.h"

#include "task.h"

#include "ready_queue.h"

#include "cpu_attr.h"
#include "cpu_status.h"
#include "m5b_trace.h"



IMPORT TCB *ctxtsk;

IMPORT TCB *schedtsk;

IMPORT TCB *tcb_table;

IMPORT ID   max_tskid;

IMPORT RDYQUE ready_queue;



extern void dispatch_to_schedtsk(void);

extern void uart_puts(const char *s);

extern void uart_putc(char c);



/* Set to 1 only for interactive UART tracing (slow); ring log needs 0 */
#define M5B_FD_VERBOSE  0

static void m5b_kernel_idle_halt(void)
{
    static int reported;

    if (!reported) {
        reported = 1;
        uart_puts("[M5b] kernel idle (no runnable tasks)\r\n");
    }
    m5b_ext_tsk_hold_int = 0;
    dispatch_to_schedtsk();
}



#if M5B_FD_VERBOSE
static void fd_put_hex64(uint64_t v)
{
    static const char hex[] = "0123456789abcdef";
    int i;

    uart_puts("0x");
    for (i = 60; i >= 0; i -= 4) {
        uart_putc(hex[(v >> i) & 0xf]);
    }
}
#endif

static void fd_dump_next(const char *tag, TCB *next)

{

    uintptr_t ssp;

    uintptr_t elr96;

    uintptr_t spsr104;



#if M5B_FD_VERBOSE

    uart_puts(tag);

    if (next == NULL) {

        uart_puts(" next=NULL\r\n");

        return;

    }

    uart_puts(" tskid=");

    fd_put_hex64((uint64_t)next->tskid);

    uart_puts(" state=");

    fd_put_hex64((uint64_t)next->state);

    uart_puts(" pri=");

    fd_put_hex64((uint64_t)next->priority);

    ssp = (uintptr_t)next->tskctxb.ssp;

    uart_puts(" ssp=");

    fd_put_hex64(ssp);

    if (ssp != 0) {

        elr96 = *(uintptr_t *)(ssp + TK2_CTX_ELR);

        spsr104 = *(uintptr_t *)(ssp + TK2_CTX_SPSR);

        uart_puts(" [+96]=");

        fd_put_hex64(elr96);

        uart_puts(" [+104]=");

        fd_put_hex64(spsr104);

    }

    uart_puts("\r\n");

#else

    (void)tag;

    (void)next;

    (void)ssp;

    (void)elr96;

    (void)spsr104;

#endif

}



static BOOL tcb_on_ready_queue(TCB *tcb)

{

    QUEUE *head = &ready_queue.tskque[tcb->priority];

    QUEUE *q;



    for (q = head->next; q != head; q = q->next) {

        if ((TCB *)q == tcb) {

            return TRUE;

        }

    }

    return FALSE;

}



void dispatch_saved_ctxtsk(TCB *tcb)

{

    if (tcb == NULL) {

        return;

    }

    m5b_log_event(M5B_LOG_SAVE, tcb, tcb->tskctxb.ssp);

    if (tcb->state == TS_READY && !tcb_on_ready_queue(tcb)) {

        make_ready(tcb);

    }

}



static TCB *find_runnable_tcb(void)

{

    TCB *best = NULL;

    ID i;



    for (i = MIN_TSKID; i <= max_tskid; i++) {

        TCB *t = &tcb_table[INDEX_TSK(i)];



        if (t->state != TS_READY) {

            continue;

        }

        if (t->tskctxb.ssp == NULL) {

            continue;

        }

        if (best == NULL || t->priority < best->priority) {

            best = t;

        }

    }

    return best;

}



/* M5 app tasks run at priority 10; init_task is 12 and stays READY on the queue */

static TCB *find_ready_at_priority(UB pri)

{

    TCB *found = NULL;

    ID i;



    for (i = MIN_TSKID; i <= max_tskid; i++) {

        TCB *t = &tcb_table[INDEX_TSK(i)];



        if (t->state != TS_READY || t->tskctxb.ssp == NULL) {

            continue;

        }

        if (t->priority != pri) {

            continue;

        }

        if (found == NULL || t->tskid < found->tskid) {

            found = t;

        }

    }

    return found;

}



static BOOL tcb_pointer_valid(TCB *t)

{

    ptrdiff_t idx;



    if (t == NULL || tcb_table == NULL) {

        return FALSE;

    }

    idx = t - tcb_table;

    if (idx < 0 || idx >= NUM_TSKID) {

        return FALSE;

    }

    if (t->tskid == 0) {

        return FALSE;

    }

    return TRUE;

}



static BOOL schedtsk_is_runnable(TCB *t)

{

    if (!tcb_pointer_valid(t)) {

        return FALSE;

    }

    if (t->state == TS_DORMANT || t->state == TS_NONEXIST) {

        return FALSE;

    }

    /* Only TS_READY (incl. RUN) may be dispatched — not TS_WAIT */
    if (t->state != TS_READY) {

        return FALSE;

    }

    if (t->tskctxb.ssp == NULL) {

        return FALSE;

    }

    return TRUE;

}



void rv64_force_dispatch(void)
{
    TCB *next;

    m5b_use_tmp_stack();
    (void)disint();

    m5b_log_event(M5B_LOG_FD, schedtsk, NULL);

    ctxtsk = NULL;

    /* Match upstream: head of global ready queue (post make_non_ready / mutex wake) */
    next = ready_queue_top(&ready_queue);
    fd_dump_next("[FD] top", next);

    if (!schedtsk_is_runnable(next)) {
        next = find_ready_at_priority(10);
        fd_dump_next("[FD] pri10", next);
    }
    if (!schedtsk_is_runnable(next)) {
        next = find_runnable_tcb();
        fd_dump_next("[FD] scan", next);
    }

    if (next != NULL) {
        m5b_frame_fixup_lr(next->tskctxb.ssp);
    }

    schedtsk = next;

    if (schedtsk == NULL) {
        m5b_kernel_idle_halt();
        return;
    }

    m5b_log_event(M5B_LOG_FD, schedtsk, schedtsk->tskctxb.ssp);
    fd_dump_next("[FD] sched", schedtsk);

    m5b_ext_tsk_hold_int = 0;
    dispatch_to_schedtsk();
}


