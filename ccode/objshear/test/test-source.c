#include <stdio.h>
#include <stdlib.h>
#include "../source.h"
#include "../defs.h"

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("usage: %s source_filename\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char* fname=argv[1];
    struct scat* scat = scat_read(fname);

    scat_print_firstlast(scat);

    scat=scat_delete(scat);
}
