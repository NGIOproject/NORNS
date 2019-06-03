#ifndef __XMALLOC_H__
#define __XMALLOC_H__

#include <sys/types.h>
#include <stdbool.h>

#pragma GCC visibility push(hidden)

#define xmalloc(__sz) \
    norns_xmalloc(__sz, true)

#define xmalloc_nz(__sz) \
    norns_xmalloc(__sz, false)

#define xfree(__p) \
    norns_xfree((void**)&(__p))

void* norns_xmalloc(size_t, bool);
void norns_xfree(void**);

#pragma GCC visibility pop

#endif /* __XMALLOC_H__ */
