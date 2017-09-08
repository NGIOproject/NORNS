#include <norns.h>

#define ERR_REMAP(n) ((n) < 0 ? -(n) : (n))

const char* const norns_errlist[NORNS_ERRMAX + 1] = {
    [ERR_REMAP(NORNS_SUCCESS)] = "Success",
    [ERR_REMAP(NORNS_ESNAFU)] = "Internal error",
    [ERR_REMAP(NORNS_EBADREQUEST)] = "Bad request",
    [ERR_REMAP(NORNS_EBADARGS)] = "Bad arguments",
    [ERR_REMAP(NORNS_ENOMEM)] = "Cannot allocate memory",
    [ERR_REMAP(NORNS_ECONNFAILED)] = "Cannot connect to daemon",
    [ERR_REMAP(NORNS_ERPCSENDFAILED)] = "Cannot send requests to daemon",
    [ERR_REMAP(NORNS_ERPCRECVFAILED)] = "Cannot receive responses from daemon",
    [ERR_REMAP(NORNS_EJOBEXISTS)] = "Job already exists",
    [ERR_REMAP(NORNS_ENOSUCHJOB)] = "Job does not exist",
    [ERR_REMAP(NORNS_ENOSUCHPROCESS)] = "Process does not exist",

    [ERR_REMAP(NORNS_ERRMAX)] = "Unknown error",

};

char*
norns_strerror(int errnum) {

    if(errnum > NORNS_ERRMAX) {
        errnum = NORNS_ERRMAX;
    }

    return (char*) norns_errlist[ERR_REMAP(errnum)];
}
