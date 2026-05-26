/* kernel/sysdepend/cpu/aarch64/kernel_syscall.c
 * M5: EL0 SVC ディスパッチ（スタブ）。番号は aarch64_syscall.h で定義予定。
 */

#include <stdint.h>

extern void uart_puts(const char *s);

int64_t knl_syscall_handler(uint32_t syscall_nr)
{
    /* 未実装 syscall は ENOSYS (-38) */
    (void)syscall_nr;
    return -38;
}
