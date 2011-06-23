#include <stdio.h>
#include <stdlib.h>
#include "../lens.h"

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("usage: %s lens_filename\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char* fname=argv[1];
    struct lcat* lcat = lcat_read(fname);

    lcat_print_firstlast(lcat);

    lcat=lcat_delete(lcat);
}
