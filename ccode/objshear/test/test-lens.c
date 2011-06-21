#include <stdio.h>
#include <stdlib.h>
#include "../lens.h"

int main(int argc, char** argv) {
    struct lcat* lcat = lcat_new(25);
    lcat = lcat_delete(lcat);
    /*
    if (argc < 2) {
        printf("usage: %s lens_filename\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char* fname=argv[1];
    struct lcat* lcat = lcat_read(fname);

    lcat_print_firstlast(lcat);

    lcat=lcat_delete(lcat);
    */
}
