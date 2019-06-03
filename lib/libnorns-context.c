#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include "libnorns-context.h"

static pthread_key_t libnorns_context_key;

static int
libnorns_create_context_key(void) {

    int err = 0;

    if((err = pthread_key_create(&libnorns_context_key, NULL)) != 0) {
        errno = err;
        return -1;
    }

    return 0;
}

static void
libnorns_delete_context_key(void) {
    pthread_key_delete(libnorns_context_key);
}

void
libnorns_create_context(void) {

    struct libnorns_context* ctx;

    if(libnorns_create_context_key() != 0) {
        fprintf(stderr, "libnorns: Failed to create thread specific key: %s\n",
                strerror(errno));
        exit(EXIT_FAILURE);
    }

    ctx = (struct libnorns_context*) pthread_getspecific(libnorns_context_key);

    if(ctx != NULL) {
        return;
    }

    ctx = (struct libnorns_context* ) malloc(sizeof(struct libnorns_context));

    if(ctx == NULL) {
        fprintf(stderr, "libnorns: Failed to allocate thread specific data\n");
        exit(EXIT_FAILURE);
    }

    memset(ctx, 0, sizeof(struct libnorns_context));

    int err = pthread_setspecific(libnorns_context_key, ctx);

    if(err != 0) {
        fprintf(stderr, "libnorns: Failed to set thread specific data\n");
        exit(EXIT_FAILURE);
    }
}

struct libnorns_context*
libnorns_get_context(void) {

    struct libnorns_context* ctx;

    ctx = (struct libnorns_context*) pthread_getspecific(libnorns_context_key);

    // this can only happen if we did not call libnorns_create_context()
    // before trying to access the context
    if(ctx == NULL) {
        fprintf(stderr, "libnorns: Failed to retrieve library context\n");
        exit(EXIT_FAILURE);
    }

    return ctx;
}

void
libnorns_destroy_context(void) {

    libnorns_delete_context_key();
}
