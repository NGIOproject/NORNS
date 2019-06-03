#if !defined(__NORNS_LIB_H__) && !defined(__NORNSCTL_LIB_H__)
#error "Never include <norns_error.h> directly; use <norns.h> or <nornsctl.h> instead."
#endif

#ifndef __NORNS_ERROR_H__
#define __NORNS_ERROR_H__ 1

#ifdef __cplusplus
extern "C" {
#endif

#define NORNS_ERRMAX 512

/** Error codes */
#define NORNS_SUCCESS              0
#define NORNS_ESNAFU              -1
#define NORNS_EBADARGS            -2
#define NORNS_EBADREQUEST         -3
#define NORNS_ENOMEM              -4
#define NORNS_ETIMEOUT            -5

/* errors about communication */
#define NORNS_ECONNFAILED         -6
#define NORNS_ERPCSENDFAILED      -7
#define NORNS_ERPCRECVFAILED      -8
#define NORNS_EACCEPTPAUSED       -9

/* errors about jobs */
#define NORNS_EJOBEXISTS         -10
#define NORNS_ENOSUCHJOB         -11

/* errors about processes */
#define NORNS_EPROCESSEXISTS     -20
#define NORNS_ENOSUCHPROCESS     -21

/* errors about namespaces */
#define NORNS_ENAMESPACEEXISTS   -30
#define NORNS_ENOSUCHNAMESPACE   -31
#define NORNS_ENAMESPACENOTEMPTY -32

/* errors about tasks */
#define NORNS_ETASKEXISTS        -40
#define NORNS_ENOSUCHTASK        -41
#define NORNS_ETOOMANYTASKS      -42
#define NORNS_ETASKSPENDING      -43

/* task status */
#define NORNS_EUNDEFINED        -100
#define NORNS_EPENDING          -101
#define NORNS_EINPROGRESS       -102
#define NORNS_EFINISHED         -103
#define NORNS_EFINISHEDWERROR   -104

/* errors resources */
#define NORNS_ERESOURCEEXISTS   -110
#define NORNS_ENOSUCHRESOURCE   -111

/* misc errors */
#define NORNS_ENOTSUPPORTED     -200
#define NORNS_ESYSTEMERROR      -201

#ifdef __cplusplus
}
#endif

#endif /* __NORNS_ERROR_H__ */
