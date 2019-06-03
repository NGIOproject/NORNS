#include <norns.h>

#define ERR_REMAP(n) ((n) < 0 ? -(n) : (n))

const char* const norns_errlist[NORNS_ERRMAX + 1] = {
    [ERR_REMAP(NORNS_SUCCESS)] = "Success",
    [ERR_REMAP(NORNS_ESNAFU)] = "Internal error",
    [ERR_REMAP(NORNS_EBADREQUEST)] = "Bad request",
    [ERR_REMAP(NORNS_EBADARGS)] = "Bad arguments",
    [ERR_REMAP(NORNS_ENOMEM)] = "Cannot allocate memory",
    [ERR_REMAP(NORNS_ETIMEOUT)] = "Timeout exceeded",

    /* communication errors */
    [ERR_REMAP(NORNS_ECONNFAILED)] = "Cannot connect to daemon",
    [ERR_REMAP(NORNS_ERPCSENDFAILED)] = "Cannot send requests to daemon",
    [ERR_REMAP(NORNS_ERPCRECVFAILED)] = "Cannot receive responses from daemon",
    [ERR_REMAP(NORNS_EACCEPTPAUSED)] = "Daemon does not accept new tasks",

    /* job errors */
    [ERR_REMAP(NORNS_EJOBEXISTS)] = "Job already exists",
    [ERR_REMAP(NORNS_ENOSUCHJOB)] = "Job does not exist",

    /* process errors */
    [ERR_REMAP(NORNS_EPROCESSEXISTS)] = "Process already exists",
    [ERR_REMAP(NORNS_ENOSUCHPROCESS)] = "Process does not exist",
    
    /* backend errors */
    [ERR_REMAP(NORNS_ENAMESPACEEXISTS)] = "Namespace already exists",
    [ERR_REMAP(NORNS_ENOSUCHNAMESPACE)] = "Namespace does not exist",
    [ERR_REMAP(NORNS_ENAMESPACENOTEMPTY)] = "Namespace is not empty",
    
    /* task errors */
    [ERR_REMAP(NORNS_ETASKEXISTS)] = "Task already exists",
    [ERR_REMAP(NORNS_ENOSUCHTASK)] = "Task does not exist",
    [ERR_REMAP(NORNS_ETOOMANYTASKS)] = "Too many pending tasks",
    [ERR_REMAP(NORNS_ETASKSPENDING)] = "There are still pending tasks",

    /* resource errors */
    [ERR_REMAP(NORNS_ERESOURCEEXISTS)] = "Resource already exists",
    [ERR_REMAP(NORNS_ENOSUCHRESOURCE)] =  "Resource does not exist",

    /* misc errors */
    [ERR_REMAP(NORNS_ENOTSUPPORTED)] = "Not supported",
    [ERR_REMAP(NORNS_ESYSTEMERROR)] = "Operating system error",

    /* fallback */
    [ERR_REMAP(NORNS_ERRMAX)] = "Unknown error",

};

char*
__norns_strerror(int errnum) {

    if(errnum > NORNS_ERRMAX) {
        errnum = NORNS_ERRMAX;
    }

    return (char*) norns_errlist[ERR_REMAP(errnum)];
}

# define weak_alias(name, aliasname) \
  extern __typeof (name) aliasname __attribute__ ((weak, alias (#name)));

weak_alias(__norns_strerror, norns_strerror);
weak_alias(__norns_strerror, nornsctl_strerror);
