#ifndef __LIBNORNSCTL_CONTEXT_H__
#define __LIBNORNSCTL_CONTEXT_H__

struct libnornsctl_context {
    const char* config_file;
    const char* api_socket;
};

#define LIBSTR "libnornsctl"
#define LIBSYMBOL(sym) libnornsctl_##sym
typedef struct libnornsctl_context libcontext_t;

void libnornsctl_create_context(void);
void libnornsctl_destroy_context(void);
struct libnornsctl_context* libnornsctl_get_context(void);

#endif /* __LIBNORNSCTL_CONTEXT_H__ */

