#include <stdio.h>
#include <stdlib.h>
#include "../config.h"

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("usage: %s config_filename\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char* fname=argv[1];
    struct config* c=read_config(fname);

    print_config(c);

    free(c);
}
