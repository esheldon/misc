#include <stdio.h>
#include <stdlib.h>
#include "../config.h"


int main(int argc, char** argv) {

    if (argc != 2 ) {
        printf("usage: test-config filename\n");
        exit(45);
    }

    const char* filename = argv[1];

    struct config pars;

    read_config(filename, pars);
    print_config(pars);
}

