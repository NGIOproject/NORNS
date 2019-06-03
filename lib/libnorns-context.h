#ifndef __LIBNORNS_CONTEXT_H__
#define __LIBNORNS_CONTEXT_H__

struct libnorns_context {
    const char* config_file;
    const char* api_socket;
};

#define LIBSTR "libnorns"
#define LIBSYMBOL(sym) libnorns_##sym
typedef struct libnorns_context libcontext_t;

void libnorns_create_context(void);
void libnorns_destroy_context(void);
struct libnorns_context* libnorns_get_context(void);

#endif /* __LIB_CONTEXT_H__ */

