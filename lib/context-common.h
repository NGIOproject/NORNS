#ifndef __CONTEXT_COMMON_H__
#define __CONTEXT_COMMON_H__

#if defined (__BUILD_LIBNORNS__)
#   include "libnorns-context.h"
#elif defined (__BUILD_LIBNORNSCTL__)
#   include "libnornsctl-context.h"
#else
#   error "Missing build target definition"
#endif

static inline libcontext_t*
get_context(void) {
    return LIBSYMBOL(get_context)();
}

#endif /* __CONTEXT_COMMON_H__ */
