#include "tk2_la64_hw.h"

#define CSR_ECFG       0x04
#define CSR_EENTRY     0x0C
#define CSR_TLBRENTRY  0x88
#define CSR_MERRENTRY  0x93
#define ECFG_TIMER_BIT (1U << 11)

extern void exception_entry(void);
extern void tlb_refill_entry(void);
extern void machine_error_entry(void);

void exception_vector_init(void)
{
    unsigned long addr;

    __asm__ volatile("csrwr %0, %1" :: "r"((unsigned long)ECFG_TIMER_BIT),
                     "i"(CSR_ECFG));

    addr = (unsigned long)&exception_entry & ~3UL;
    __asm__ volatile("csrwr %0, %1" :: "r"(addr), "i"(CSR_EENTRY));

    addr = (unsigned long)&tlb_refill_entry & ~3UL;
    __asm__ volatile("csrwr %0, %1" :: "r"(addr), "i"(CSR_TLBRENTRY));

    addr = (unsigned long)&machine_error_entry & ~3UL;
    __asm__ volatile("csrwr %0, %1" :: "r"(addr), "i"(CSR_MERRENTRY));
}

void irq_enable(void)
{
    unsigned long crmd;

    __asm__ volatile("csrrd %0, %1" : "=r"(crmd) : "i"(0x00));
    crmd |= (1UL << 2);
    __asm__ volatile("csrwr %0, %1" :: "r"(crmd), "i"(0x00) : "memory");
}

void irq_disable(void)
{
    unsigned long crmd;

    __asm__ volatile("csrrd %0, %1" : "=r"(crmd) : "i"(0x00));
    crmd &= ~(1UL << 2);
    __asm__ volatile("csrwr %0, %1" :: "r"(crmd), "i"(0x00) : "memory");
}
