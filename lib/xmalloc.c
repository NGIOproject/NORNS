#include <unistd.h>
#include <stdlib.h>

#include "xmalloc.h"

void*
norns_xmalloc(size_t size, bool clear) {
	void* p;

	if(size <= 0) {
		return NULL;
    }

	if(clear) {
		p = calloc(1, size);
    }
	else {
		p = malloc(size);
    }

    return p;
}

void
norns_xfree(void** p) {
    if(*p != NULL) {
        free(*p);
        *p = NULL;
    }
}
