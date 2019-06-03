#ifndef __XSTRING_H__
#define __XSTRING_H__

#include <sys/types.h>
#include <stdbool.h>

#pragma GCC visibility push(hidden)

// maximum string length supported for security reasons (including '\0')
// any input strings larger than MAX_STRING_LENGTH will be truncated
// (if this becomes a problem at some point, just enlarge it)
#define XMAX_STRING_LENGTH   4096
#define XMIN(a, b) \
    (a) < (b) ? (a) : (b)

char* xstrdup(const char* str);
int xstrcnmp(const char* s1, const char* s2);

#pragma GCC visibility pop

#endif /* __XSTRING_H__ */
