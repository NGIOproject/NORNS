#ifndef __CONF_PARSER_H__
#define __CONF_PARSER_H__

#include <unistd.h>

struct kvpair {
    const char* key;
    const char* val; 
};

int parse_config_file(const char* config_file, struct kvpair valid_opts[],
                      size_t num_valid_opts);

#endif /* __CONF_PARSER_H__ */

