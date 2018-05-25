/*************************************************************************
 * Copyright (C) 2017-2018 Barcelona Supercomputing Center               *
 *                         Centro Nacional de Supercomputacion           *
 * All rights reserved.                                                  *
 *                                                                       *
 * This file is part of the NORNS Data Scheduler, a service that allows  *
 * other programs to start, track and manage asynchronous transfers of   *
 * data resources transfers requests between different storage backends. *
 *                                                                       *
 * See AUTHORS file in the top level directory for information           *
 * regarding developers and contributors.                                *
 *                                                                       *
 * The NORNS Data Scheduler is free software: you can redistribute it    *
 * and/or modify it under the terms of the GNU Lesser General Public     *
 * License as published by the Free Software Foundation, either          *
 * version 3 of the License, or (at your option) any later version.      *
 *                                                                       *
 * The NORNS Data Scheduler is distributed in the hope that it will be   *
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty   *
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU  *
 * Lesser General Public License for more details.                       *
 *                                                                       *
 * You should have received a copy of the GNU Lesser General             *
 * Public License along with the NORNS Data Scheduler.  If not, see      *
 * <http://www.gnu.org/licenses/>.                                       *
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
