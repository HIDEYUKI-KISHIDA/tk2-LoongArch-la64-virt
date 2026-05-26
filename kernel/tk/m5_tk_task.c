/* kernel/tk/m5_tk_task.c — M5: tk_cre_tsk / tk_sta_tsk (dual task) */

#include <tk/tk_m5_api.h>
#include <stdint.h>
#include <libstr.h>

extern void uart_puts(const char *s);
extern void uart_putc(char c);
extern void uart_put_er(ER ercd);

#define M5_AB_ROUNDS  5

#if defined(M5B_TKERNEL)
#define M5_LOC_MTX(id)  tk_loc_mtx((id), TMO_FEVR)
#else
#define M5_LOC_MTX(id)  tk_loc_mtx(id)
#endif

static ID g_mtx;

static ER m5_lock(void)
{
    ER ercd = M5_LOC_MTX(g_mtx);

    if (ercd != E_OK) {
        uart_puts("[M5] loc_mtx failed");
        uart_put_er(ercd);
        uart_puts("\r\n");
    }
    return ercd;
}

static void task_a(void)
{
    unsigned int i;

    uart_puts("[task_a] start\r\n");
    for (i = 0; i < M5_AB_ROUNDS; i++) {
        if (m5_lock() != E_OK) {
            break;
        }
        uart_putc('A');
        tk_unl_mtx(g_mtx);
        tk_rot_rdq(TPRI_SELF);
    }
    uart_puts("\r\n[task_a] done\r\n");
    tk_ext_tsk();
}

static void task_b(void)
{
    unsigned int i;

    uart_puts("[task_b] start\r\n");
    for (i = 0; i < M5_AB_ROUNDS; i++) {
        if (m5_lock() != E_OK) {
            break;
        }
        uart_putc('B');
        tk_unl_mtx(g_mtx);
        tk_rot_rdq(TPRI_SELF);
    }
    uart_puts("\r\n[task_b] done\r\n");
    tk_ext_tsk();
}

void m5_run_tk_task(void)
{
    T_CTSK ctsk;
    ID id_a, id_b;

#if defined(M5B_TKERNEL)
    T_CMTX mtx;

    memset(&ctsk, 0, sizeof(ctsk));
    memset(&mtx, 0, sizeof(mtx));
#else
    T_MTXCB mtx;
#endif

    uart_puts("\r\n[M5] tk_cre_tsk (dual task)\r\n");

    mtx.mtxatr = TA_TFIFO;
    mtx.ceilpri = 10;
    g_mtx = tk_cre_mtx(&mtx);
    if (g_mtx <= 0) {
        uart_puts("[M5] tk_cre_mtx failed\r\n");
        return;
    }

    ctsk.exinf = 0;
    ctsk.tskatr = TA_HLNG | TA_RNG0;
    ctsk.itskpri = 10;
    ctsk.stksz = 8192;

    ctsk.task = task_a;
    id_a = tk_cre_tsk(&ctsk);
    if (id_a <= 0) {
        uart_puts("[M5] tk_cre_tsk(A) failed\r\n");
        return;
    }
    uart_puts("[M5] tk_cre_tsk(A) OK\r\n");

    ctsk.task = task_b;
    id_b = tk_cre_tsk(&ctsk);
    if (id_b <= 0) {
        uart_puts("[M5] tk_cre_tsk(B) failed\r\n");
        return;
    }
    uart_puts("[M5] tk_cre_tsk(B) OK\r\n");

    if (tk_dis_dsp() != E_OK) {
        uart_puts("[M5] tk_dis_dsp failed\r\n");
        return;
    }
    if (tk_sta_tsk(id_a, 0) != E_OK) {
        uart_puts("[M5] tk_sta_tsk(A) failed\r\n");
        return;
    }
    if (tk_sta_tsk(id_b, 0) != E_OK) {
        uart_puts("[M5] tk_sta_tsk(B) failed\r\n");
        return;
    }
    uart_puts("[M5] tk_sta_tsk OK, dispatching...\r\n");
    if (tk_ena_dsp() != E_OK) {
        uart_puts("[M5] tk_ena_dsp failed\r\n");
        return;
    }

    uart_puts("[M5] COMPLETE (returned to idle)\r\n");
}
