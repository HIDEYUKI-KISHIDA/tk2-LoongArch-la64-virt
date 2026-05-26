#include <basic.h>
#include <tk/tkernel.h>

#define CacheLineSZ 64

EXPORT INT GetCacheLineSize(void)
{
    return CacheLineSZ;
}

EXPORT void FlushCache(W mode, UW laddr, INT len)
{
    (void)mode;
    (void)laddr;
    (void)len;
}

EXPORT void InvalidateCache(W mode, UW laddr, INT len)
{
    (void)mode;
    (void)laddr;
    (void)len;
}
