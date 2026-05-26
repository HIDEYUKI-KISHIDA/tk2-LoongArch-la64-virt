#include <tk/typedef.h>

EXPORT void low_pow(void)
{
    asm volatile("idle 0");
}

EXPORT void off_pow(void)
{
    asm volatile("idle 0");
}
