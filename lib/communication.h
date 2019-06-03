#ifndef __DAEMON_COMMUNICATION_H__
#define __DAEMON_COMMUNICATION_H__

#include "requests.h"

#pragma GCC visibility push(hidden)

norns_error_t send_submit_request(norns_iotask_t* task);
norns_error_t send_control_command_request(nornsctl_command_t cmd, void* args);
norns_error_t send_status_request(norns_iotask_t* task, norns_stat_t* stats);
norns_error_t send_job_request(norns_msgtype_t type, uint32_t jobid, 
                               nornsctl_job_t* job);
norns_error_t send_process_request(norns_msgtype_t type, uint32_t jobid, 
                                   uid_t uid, gid_t gid, pid_t pid);
norns_error_t send_namespace_request(norns_msgtype_t type, const char* nsid, 
                                     nornsctl_backend_t* backend);
norns_error_t send_control_status_request(nornsctl_stat_t* stats);

#pragma GCC visibility pop

#endif /* __DAEMON_COMMUNICATION_H__ */
