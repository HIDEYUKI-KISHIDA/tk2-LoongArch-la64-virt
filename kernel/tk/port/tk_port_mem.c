/* tk_port_mem.c — M5b bump allocator (Imalloc) for kernel objects */

#include <sys/imalloc.h>
#include <tk/errno.h>
#include <libstr.h>
#include <stdint.h>

#define TK_PORT_HEAP_SIZE  (2U * 1024U * 1024U)

static uint8_t g_heap[TK_PORT_HEAP_SIZE] __attribute__((aligned(16)));
static uintptr_t g_heap_used;

void *Imalloc(size_t size)
{
    uintptr_t base = (uintptr_t)g_heap;
    uintptr_t end = base + TK_PORT_HEAP_SIZE;
    uintptr_t off, next;

    if (size == 0) {
        return 0;
    }
    off = (g_heap_used + 15U) & ~(uintptr_t)15U;
    next = off + size;
    if (next < off || base + next > end) {
        return 0;
    }
    g_heap_used = next;
    return (void *)(base + off);
}

void Ifree(void *ptr)
{
    (void)ptr;
}

void *IAmalloc(size_t size, UINT attr)
{
    (void)attr;
    return Imalloc(size);
}

void *IAcalloc(size_t nmemb, size_t size, UINT attr)
{
    void *p;

    (void)attr;
    p = Imalloc(nmemb * size);
    if (p != 0) {
        memset(p, 0, nmemb * size);
    }
    return p;
}

void IAfree(void *ptr, UINT attr)
{
    (void)attr;
    Ifree(ptr);
}

void *Icalloc(size_t nmemb, size_t size)
{
    return IAcalloc(nmemb, size, 0);
}

ER init_Imalloc(void)
{
    g_heap_used = 0;
    return E_OK;
}
