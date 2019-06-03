#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include "libnornsctl-context.h"

static pthread_key_t libnornsctl_context_key;

static int
libnornsctl_create_context_key(void) {

    int err = 0;

    if((err = pthread_key_create(&libnornsctl_context_key, NULL)) != 0) {
        errno = err;
        return -1;
    }

    return 0;
}

static void
libnornsctl_delete_context_key(void) {
    pthread_key_delete(libnornsctl_context_key);
}

void
libnornsctl_create_context(void) {

    struct libnornsctl_context* ctx;

    if(libnornsctl_create_context_key() != 0) {
        fprintf(stderr, "libnorns: Failed to create thread specific key: %s\n",
                strerror(errno));
        exit(EXIT_FAILURE);
    }

    ctx = (struct libnornsctl_context*) pthread_getspecific(libnornsctl_context_key);

    if(ctx != NULL) {
        return;
    }

    ctx = (struct libnornsctl_context* ) malloc(sizeof(struct libnornsctl_context));

    if(ctx == NULL) {
        fprintf(stderr, "libnorns: Failed to allocate thread specific data\n");
        exit(EXIT_FAILURE);
    }

    memset(ctx, 0, sizeof(struct libnornsctl_context));

    int err = pthread_setspecific(libnornsctl_context_key, ctx);

    if(err != 0) {
        fprintf(stderr, "libnorns: Failed to set thread specific data\n");
        exit(EXIT_FAILURE);
    }
}

struct libnornsctl_context*
libnornsctl_get_context(void) {

    struct libnornsctl_context* ctx;

    ctx = (struct libnornsctl_context*) pthread_getspecific(libnornsctl_context_key);

    // this can only happen if we did not call libnornsctl_create_context()
    // before trying to access the context
    if(ctx == NULL) {
        fprintf(stderr, "libnorns: Failed to retrieve library context\n");
        exit(EXIT_FAILURE);
    }

    return ctx;
}

void
libnornsctl_destroy_context(void) {
    libnornsctl_delete_context_key();
}
