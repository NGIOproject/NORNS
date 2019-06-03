#ifndef __NORNS_RPC_H__
#define __NORNS_RPC_H__ 1

#include <arpa/inet.h>

// #define NORNS_RPC_TRANSFER      0x1000000
// #define NORNS_RPC_WAIT          0x1000001
// #define NORNS_RPC_CANCEL        0x1000002
// #define NORNS_RPC_RETURN        0x1000003
// #define NORNS_RPC_PROGRESS      0x1000004
// #define NORNS_RPC_ERROR         0x1000005
// 
// 
// #define NORNS_RPC_COMMAND       0x2000000
// #define NORNS_RPC_REGISTER_JOB  0x2000001
// #define NORNS_RPC_UPDATE_JOB    0x2000002
// #define NORNS_RPC_REMOVE_JOB    0x2000003

#define NORNS_RPC_HEADER_LENGTH sizeof(uint64_t) 

static inline uint64_t htonll(uint64_t x) {
    return ((1==htonl(1)) ? (x) : ((uint64_t)htonl((x) & 0xFFFFFFFF) << 32) | htonl((x) >> 32));
}

static inline uint64_t ntohll(uint64_t x) {
    return ((1==ntohl(1)) ? (x) : ((uint64_t)ntohl((x) & 0xFFFFFFFF) << 32) | ntohl((x) >> 32));
}

#endif /* __NORNS_RPC_H__ */
