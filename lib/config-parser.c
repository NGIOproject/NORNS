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

#include <yaml.h>
#include <stdbool.h>
#include "defaults.h"
#include "xstring.h"
#include "log.h"
#include "config-parser.h"

int
parse_config_file(const char* config_file, struct kvpair valid_opts[], 
                  size_t num_valid_opts) {

    int rv = 0;
    yaml_parser_t parser;
    yaml_token_t token;

    if(!yaml_parser_initialize(&parser)) {
        ERR("yaml_parser_initialize");
        return -1;
    }

    FILE* fh = fopen(config_file, "r");

    if(fh == NULL) {
        ERR("!fopen");
        rv = -1;
        goto cleanup;
    }

    yaml_parser_set_input_file(&parser, fh);

    enum { EXPECT_KEY, 
           EXPECT_VALUE } scan_state = EXPECT_KEY;
    const char** datap = NULL;

    do {

        if(yaml_parser_scan(&parser, &token) == 0) {
            ERR("yaml_parser_scan");
            continue;
        }

        switch(token.type) {

            case YAML_KEY_TOKEN:
                scan_state = EXPECT_KEY;
                break;

            case YAML_VALUE_TOKEN:
                scan_state = EXPECT_VALUE;
                break;
            
            case YAML_SCALAR_TOKEN:
            {
                char* tk = (char*) token.data.scalar.value;

                switch(scan_state) {
                    case EXPECT_KEY:

                        datap = NULL;

                        for(size_t i = 0; i < num_valid_opts; ++i) {
                            if(!strcmp(tk, valid_opts[i].key)) {
                                datap = &valid_opts[i].val;
                                break;
                            }
                        }
                        break;

                    case EXPECT_VALUE:
                        if(datap != NULL) {
                            *datap = xstrdup(tk);

                            if(*datap == NULL) {
                                ERR("!xstrdup");
                            }
                        }
                        break;

                    default:
                        break;
                }
                break;
            }

            default:
                break;
        }

        if(token.type != YAML_STREAM_END_TOKEN) {
            yaml_token_delete(&token);
        }
    } while(token.type != YAML_STREAM_END_TOKEN);

    yaml_token_delete(&token);

cleanup:
    yaml_parser_delete(&parser);

    if(fh != NULL) {
        fclose(fh);
    }

    return rv;
}
