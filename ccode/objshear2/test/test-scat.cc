#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include "../scat.h"


int main(int argc, char** argv) {

    if (argc != 2 ) {
        printf("usage: test-scat filename\n");
        exit(45);
    }

    const char* filename = argv[1];

    std::vector<struct source> scat;

    read_source(filename, scat);

    print_source_firstlast(scat);

    add_source_distances(100.0, 0.3, scat);

    print_source_firstlast(scat);
}

