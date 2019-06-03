#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>


#include <norns.h>

#if 0
void print_iotd(struct norns_iotd* iotdp){
    fprintf(stdout, "iotd -> struct nornds_iotd {\n");
    fprintf(stdout, "  io_taskid = %d;\n", iotdp->io_taskid);
    fprintf(stdout, "};\n");
}
#endif


int main() {
#if 0
    struct norns_iotd iotd = {
        .io_taskid = 0,
    };

    fprintf(stdout, "calling norns_transfer(&iotd)\n");
    print_iotd(&iotd);

    if(norns_transfer(&iotd) != 0) {
    	fprintf(stderr, "norns_transfer() error: %s \n", strerror(errno));
    	exit(EXIT_FAILURE);
    }

    fprintf(stdout, "norns_transfer() succeeded!\n");
    fprintf(stdout, "output from submission:\n");
    print_iotd(&iotd);
#endif

    return 0;
}
