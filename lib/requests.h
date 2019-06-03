#ifndef __REQUESTS_H__
#define __REQUESTS_H__

#pragma GCC visibility push(hidden)

#include "messages.pb-c.h"
#include "norns.h"

typedef enum {
    /* iotasks */
    NORNS_IOTASK_SUBMIT,
    NORNS_IOTASK_STATUS,

    NORNSCTL_GLOBAL_STATUS,

    /* control commands */
    NORNSCTL_COMMAND,

    NORNS_PING,

    /* jobs */
    NORNS_JOB_REGISTER,
    NORNS_JOB_UPDATE,
    NORNS_JOB_UNREGISTER,
    /* processes */
    NORNS_PROCESS_ADD,
    NORNS_PROCESS_REMOVE,
    /* namespaces */
    NORNS_NAMESPACE_REGISTER,
    NORNS_NAMESPACE_UPDATE,
    NORNS_NAMESPACE_UNREGISTER,
    /* other */
    NORNS_BAD_RPC
} norns_msgtype_t;

typedef struct {
    void* b_data;
    size_t b_size;
} norns_msgbuffer_t;

#define MSGBUFFER_INIT() \
{   .b_data = 0, \
    .b_size = 0 \
}

typedef struct {
    norns_msgtype_t r_type;
    int r_error_code;
    union {
        size_t r_taskid;
        struct {
            norns_status_t r_status;
            norns_error_t r_task_error;
            int r_errno;
        };
        struct {
            uint32_t r_running_tasks;
            uint32_t r_pending_tasks;
            double r_eta;
        };
    };
} norns_response_t;

int pack_to_buffer(norns_msgtype_t type, norns_msgbuffer_t* buf, ...);
int unpack_from_buffer(norns_msgbuffer_t* buf, norns_response_t* response);

#pragma GCC visibility pop

#endif /* __REQUESTS_H__ */
