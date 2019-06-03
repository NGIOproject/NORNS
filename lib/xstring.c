#include <unistd.h>
#include <string.h>
#include "xmalloc.h"
#include "xstring.h"


char*
xstrdup(const char* str) {

    size_t sz;
    char* res;

    if(str == NULL) {
        return NULL;
    }

    sz = XMIN(strlen(str) + 1, XMAX_STRING_LENGTH);
    res = (char*) xmalloc(sz);
    strncpy(res, str, sz);
    res[sz-1] = '\0';

    return res;
}

int
xstrcnmp(const char* s1, const char* s2) {

    if(s1 == NULL && s2 == NULL) {
        return 0;
    }
    else if(s1 == NULL) {
        return -1;
    }
    else if(s2 == NULL) {
        return 1;
    }
    else {
        size_t sz;
        sz = XMIN(strlen(s1), strlen(s2));
        sz = XMIN(sz, XMAX_STRING_LENGTH);

        return strncmp(s1, s2, sz);
    }
    
}

