#ifndef __NORNS_LOG_H__
#define __NORNS_LOG_H__ 1

#include <stdio.h>

#ifdef __NORNS_DEBUG__
#define ERR(...)\
    log_error(__FILE__, __LINE__, __func__, __VA_ARGS__)
#define DBG(...)\
    log_debug(__FILE__, __LINE__, __func__, __VA_ARGS__)
#else
#define ERR(...)
#define DBG(...)
#endif /* __NORNS_DEBUG__ */

#define FATAL(...)\
    log_fatal(__FILE__, __LINE__, __func__, __VA_ARGS__)

void log_init(const char* prefix, FILE* outfp);
void log_error(const char* file, int line, const char* func, 
               const char* fmt, ...);
void log_debug(const char* file, int line, const char* func, 
               const char* fmt, ...);
void log_fatal(const char* file, int line, const char* func, 
               const char* fmt, ...);

#endif /* __NORNS_LOG_H__ */
