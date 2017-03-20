/* * Copyright (C) 2017 Barcelona Supercomputing Center
 *                    Centro Nacional de Supercomputacion
 *
 * This file is part of the Data Scheduler, a daemon for tracking and managing
 * requests for asynchronous data transfer in a hierarchical storage environment.
 *
 * See AUTHORS file in the top level directory for information
 * regarding developers and contributors.
 *
 * The Data Scheduler is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Data Scheduler is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Data Scheduler.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>


#include <norns.h>

void print_iotd(struct norns_iotd* iotdp){
    fprintf(stdout, "iotd -> struct nornds_iotd {\n");
    fprintf(stdout, "  ni_tid = %d;\n", iotdp->ni_tid);
    fprintf(stdout, "};\n");
}


int main() {
    struct norns_iotd iotd = {
        .ni_tid = 0,
    };

    fprintf(stdout, "calling norns_transfer(&iotd)\n");
    print_iotd(&iotd);

    if(norns_transfer(&iotd) != 0) {
    	fprintf(stderr, "norns_transfer error: %s \n", strerror(errno));
    	exit(EXIT_FAILURE);
    }

    fprintf(stdout, "norns_transfer() succeeded!\n");
    fprintf(stdout, "output from submission:\n");
    print_iotd(&iotd);

    return 0;
}
