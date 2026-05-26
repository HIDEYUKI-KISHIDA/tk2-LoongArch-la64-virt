#include "kernel.h"

EXPORT ER tkdev_initialize(void)
{
    return E_OK;
}

EXPORT void tkdev_exit(void)
{
    for (;;) {
        asm volatile("idle 0");
    }
}
