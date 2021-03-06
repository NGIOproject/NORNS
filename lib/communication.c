/************************************************************************* 
 * Copyright (C) 2017-2019 Barcelona Supercomputing Center               *
 *                         Centro Nacional de Supercomputacion           *
 * All rights reserved.                                                  *
 *                                                                       *
 * This file is part of NORNS, a service that allows other programs to   *
 * start, track and manage asynchronous transfers of data resources      *
 * between different storage backends.                                   *
 *                                                                       *
 * See AUTHORS file in the top level directory for information regarding *
 * developers and contributors.                                          *
 *                                                                       *
 * This software was developed as part of the EC H2020 funded project    *
 * NEXTGenIO (Project ID: 671951).                                       *
 *     www.nextgenio.eu                                                  *
 *                                                                       *
 * Permission is hereby granted, free of charge, to any person obtaining *
 * a copy of this software and associated documentation files (the       *
 * "Software"), to deal in the Software without restriction, including   *
 * without limitation the rights to use, copy, modify, merge, publish,   *
 * distribute, sublicense, and/or sell copies of the Software, and to    *
 * permit persons to whom the Software is furnished to do so, subject to *
 * the following conditions:                                             *
 *                                                                       *
 * The above copyright notice and this permission notice shall be        *
 * included in all copies or substantial portions of the Software.       *
 *                                                                       *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       *
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    *
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND                 *
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS   *
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN    *
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN     *
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE      *
 * SOFTWARE.                                                             *
 *************************************************************************/

#include <stdio.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>
#include <stdarg.h>

#include "norns.h"
#include "nornsctl.h"
#include <norns-rpc.h>

#include "context-common.h"
#include "defaults.h"
#include "messages.pb-c.h"
#include "requests.h"
#include "communication.h"
#include "xmalloc.h"
#include "log.h"

static int connect_to_daemon(const char* socket_path);
static int send_message(int conn, const norns_msgbuffer_t* buffer);
static int recv_message(int conn, norns_msgbuffer_t* buffer);
static ssize_t recv_data(int conn, void* data, size_t size);
static ssize_t send_data(int conn, const void* data, size_t size);
static void print_hex(void* buffer, size_t bytes) __attribute__((unused));
static norns_error_t send_request(norns_msgtype_t type, norns_response_t* resp, ...);

norns_error_t
send_submit_request(norns_iotask_t* task) {

    int res;
    norns_response_t resp;

    // XXX add missing checks: e.g. validate src resource
    if(task->t_id != 0 || 
       (task->t_op != NORNS_IOTASK_COPY &&
        task->t_op != NORNS_IOTASK_MOVE && 
        task->t_op != NORNS_IOTASK_REMOVE)) {
        ERR("Invalid fields detected in norns_iotask_t");
        return NORNS_EBADARGS;
    }

    if((res = send_request(NORNS_IOTASK_SUBMIT, &resp, task)) 
            != NORNS_SUCCESS) {
        return res;
    }

    if(resp.r_type != NORNS_IOTASK_SUBMIT) {
        return NORNS_ESNAFU;
    }

    task->t_id = resp.r_taskid;

    return resp.r_error_code;
}

norns_error_t
send_control_command_request(nornsctl_command_t cmd, void* args) {

    int res;
    norns_response_t resp;

    if((res = send_request(NORNSCTL_COMMAND, &resp, cmd, args)) 
            != NORNS_SUCCESS) {
        return res;
    }

    if(resp.r_type != NORNSCTL_COMMAND) {
        return NORNS_ESNAFU;
    }

    return resp.r_error_code;
}


norns_error_t
send_status_request(norns_iotask_t* task, norns_stat_t* stats) {

    int res;
    norns_response_t resp;

    if(task->t_id == 0) {
        return NORNS_EBADARGS;
    }

    if((res = send_request(NORNS_IOTASK_STATUS, &resp, task)) 
            != NORNS_SUCCESS) {
        return res;
    }

    if(resp.r_type != NORNS_IOTASK_STATUS) {
        return NORNS_ESNAFU;
    }

    stats->st_status = resp.r_status;
    stats->st_task_error = resp.r_task_error;
    stats->st_sys_errno = resp.r_errno;

    return resp.r_error_code;
}

norns_error_t
send_control_status_request(nornsctl_stat_t* stats) {

    int res;
    norns_response_t resp;

    if((res = send_request(NORNSCTL_GLOBAL_STATUS, &resp)) 
            != NORNS_SUCCESS) {
        return res;
    }

    if(resp.r_type != NORNSCTL_GLOBAL_STATUS) {
        return NORNS_ESNAFU;
    }

    stats->st_running_tasks = resp.r_running_tasks;
    stats->st_pending_tasks = resp.r_pending_tasks;
    stats->st_eta = resp.r_eta;

    return resp.r_error_code;
}

norns_error_t
send_job_request(norns_msgtype_t type, uint32_t jobid, nornsctl_job_t* job) {

    int res;
    norns_response_t resp;

    if((res = send_request(type, &resp, jobid, job)) 
            != NORNS_SUCCESS) {
        return res;
    }

    if(resp.r_type != type) {
        return NORNS_ESNAFU;
    }

    return resp.r_error_code;
}


norns_error_t
send_process_request(norns_msgtype_t type, uint32_t jobid, 
                     uid_t uid, gid_t gid, pid_t pid) {

    int res;
    norns_response_t resp;

    if((res = send_request(type, &resp, jobid, uid, gid, pid)) 
            != NORNS_SUCCESS) {
        return res;
    }

    if(resp.r_type != type) {
        return NORNS_ESNAFU;
    }

    return resp.r_error_code;
}

norns_error_t
send_namespace_request(norns_msgtype_t type, const char* nsid, 
                       nornsctl_backend_t* ns) {

    int res;
    norns_response_t resp;

    if((res = send_request(type, &resp, nsid, ns)) 
            != NORNS_SUCCESS) {
        return res;
    }

    if(resp.r_type != type) {
        return NORNS_ESNAFU;
    }

    return resp.r_error_code;
}

static int
send_request(norns_msgtype_t type, norns_response_t* resp, ...) {

    int res;
    int conn = -1;
    norns_msgbuffer_t req_buf = MSGBUFFER_INIT();
    norns_msgbuffer_t resp_buf = MSGBUFFER_INIT();

    libcontext_t* ctx = get_context();

#ifdef __NORNS_DEBUG__
    libcontext_t zero_ctx;
    memset(&zero_ctx, 0, sizeof(zero_ctx));

    if(memcmp(ctx, &zero_ctx, sizeof(zero_ctx)) == 0) {
        FATAL("Library context not correctly initialized.\n"
              "    NORNS_DEBUG_CONFIG_FILE_OVERRIDE may have been set without "
              "calling\n" 
              "    libnorns_reload_config_file() or "
              "libnornsctl_reload_config_file()");
    }
#endif

    va_list ap;
    va_start(ap, resp);

    // fetch args and pack them into a buffer
    switch(type) {
        case NORNS_IOTASK_SUBMIT:
        case NORNS_IOTASK_STATUS:
        {
            const norns_iotask_t* task =
                va_arg(ap, const norns_iotask_t*);

            if((res = pack_to_buffer(type, &req_buf, task)) 
                    != NORNS_SUCCESS) {
                return res;
            }

            break;
        }

        case NORNSCTL_COMMAND:
        {
            const nornsctl_command_t cmd = va_arg(ap, nornsctl_command_t);
            const void* args = va_arg(ap, const void*);

            if((res = pack_to_buffer(type, &req_buf, cmd, args)) 
                    != NORNS_SUCCESS) {
                return res;
            }
            break;
        }

        case NORNSCTL_GLOBAL_STATUS:
        case NORNS_PING:
        {
            if((res = pack_to_buffer(type, &req_buf)) != NORNS_SUCCESS) {
                return res;
            }

            break;
        }

        case NORNS_JOB_REGISTER:
        case NORNS_JOB_UPDATE:
        case NORNS_JOB_UNREGISTER:
        {
            const uint32_t jobid =
                va_arg(ap, const uint32_t);
            const nornsctl_job_t* job =
                va_arg(ap, const nornsctl_job_t*);

            if((res = pack_to_buffer(type, &req_buf, jobid, job)) 
                    != NORNS_SUCCESS) {
                return res;
            }

            break;
        }

        case NORNS_PROCESS_ADD:
        case NORNS_PROCESS_REMOVE:
        {
            const uint32_t jobid =
                va_arg(ap, const uint32_t);
            const uid_t uid =
                va_arg(ap, const uid_t);
            const gid_t gid =
                va_arg(ap, const gid_t);
            const pid_t pid =
                va_arg(ap, const pid_t);

            if((res = pack_to_buffer(type, &req_buf, jobid, uid, gid, pid)) 
                    != NORNS_SUCCESS) {
                return res;
            }

            break;
        }

        case NORNS_NAMESPACE_REGISTER:
        case NORNS_NAMESPACE_UPDATE:
        case NORNS_NAMESPACE_UNREGISTER:
        {
            const char* const nsid =
                va_arg(ap, const char* const);
            const nornsctl_backend_t* backend = 
                va_arg(ap, const nornsctl_backend_t*);

            if((res = pack_to_buffer(type, &req_buf, nsid, backend)) 
                    != NORNS_SUCCESS) {
                return res;
            }

            break;
        }

        default:
            return NORNS_ESNAFU;

    }

    // connect to urd
    conn = connect_to_daemon(ctx->api_socket);

    if(conn == -1) {
        res = NORNS_ECONNFAILED;
        goto cleanup_on_error;
    }

	// connection established, send the message
	if(send_message(conn, &req_buf) < 0) {
	    res = NORNS_ERPCSENDFAILED;
	    goto cleanup_on_error;
    }

    // wait for the urd's response
    if(recv_message(conn, &resp_buf) < 0) {
        res = NORNS_ERPCRECVFAILED;
	    goto cleanup_on_error;
    }

    if((res = unpack_from_buffer(&resp_buf, resp)) != NORNS_SUCCESS) {
	    goto cleanup_on_error;
    }

    va_end(ap);

    close(conn);

    return res;

cleanup_on_error:
    va_end(ap);

    if(conn != -1) {
        close(conn);
    }

    return res;
}



static int 
connect_to_daemon(const char* socket_path) {

    if(socket_path == NULL) {
        return -1;
    }

	struct sockaddr_un server;

	int sfd = socket(AF_UNIX, SOCK_STREAM, 0);

	if (sfd < 0) {
	    return -1;
	}

	server.sun_family = AF_UNIX;
	strncpy(server.sun_path, socket_path, sizeof(server.sun_path));
	server.sun_path[sizeof(server.sun_path)-1] = '\0';

	if (connect(sfd, (struct sockaddr *) &server, sizeof(struct sockaddr_un)) < 0) {
	    ERR("!connect");
	    return -1;
    }

    return sfd;
}

static int
send_message(int conn, const norns_msgbuffer_t* buffer) {

    if(buffer == NULL || buffer->b_data == NULL || buffer->b_size == 0) {
        return -1;
    }

    const void* msg_data = buffer->b_data;
    size_t msg_length = buffer->b_size;


    // transform the message size into network order and send it 
    // before the actual data
    uint64_t prefix = htonll(msg_length);

    assert(sizeof(prefix) == NORNS_RPC_HEADER_LENGTH); 

	if(send_data(conn, &prefix, sizeof(prefix)) < 0) {
        return -1;
    }

	if(send_data(conn, msg_data, msg_length) < 0) {
        return -1;
    }

    return 0;
}

static int 
recv_message(int conn, norns_msgbuffer_t* buffer) {

    // first of all read the message prefix and decode it 
    // so that we know how much data to receive
    uint64_t prefix = 0;

    if(recv_data(conn, &prefix, sizeof(prefix)) < 0) {
        goto recv_error;
    }

//    print_hex(&prefix, sizeof(prefix));

    size_t msg_size = ntohll(prefix);

    if(msg_size == 0) {
        goto recv_error;
    }

    void* msg_data = xmalloc_nz(msg_size);

    if(msg_data == NULL) {
        goto recv_error;
    }

    if(recv_data(conn, msg_data, msg_size) < 0) {
        xfree(msg_data);
        goto recv_error;
    }

//    print_hex(msg_data, msg_size);

    buffer->b_data = msg_data;
    buffer->b_size = msg_size;

    return 0;

recv_error:
    buffer->b_data = NULL;
    buffer->b_size = 0;

    return -1;
}

static ssize_t 
recv_data(int conn, void* data, size_t size) {

    size_t brecvd = 0; // bytes received
    size_t bleft = size; // bytes left to receive
    ssize_t n = 0;

	while(brecvd < size) {
		n = read(conn, data + brecvd, bleft);

//		fprintf(stdout, "read %zd\n", n);

		if(n == -1 || n == 0) {
		    if(errno == EINTR) {
		        continue;
            }
			break;
		}

        brecvd += n;
        bleft -= n;
	}

	return (n == -1 ? n : (ssize_t) brecvd);
}


static ssize_t
send_data(int conn, const void* data, size_t size) {

	size_t bsent = 0;	// bytes sent
	size_t bleft = size;// bytes left to send
	ssize_t n = 0;

	// send() might not send all the bytes we ask it to,
	// because the kernel can decide not to send all the
	// data out in one chunk
	while(bsent < size) {
		n = write(conn, data + bsent, bleft);

		if(n == -1) {
		    if(errno == EINTR) {
		        continue;
            }
			break;
		}

        bsent += n;
        bleft -= n;
	}

	return (n == -1 ? n : (ssize_t) bsent);
}

static void 
print_hex(void* buffer, size_t bytes) {

    unsigned char* p = (unsigned char*) buffer;

    fprintf(stdout, "<< ");

    for(size_t i = 0; i < bytes; ++i) {
        fprintf(stdout, "%02x ", (int) p[i]);
    }

    fprintf(stdout, " >>\n");
}

