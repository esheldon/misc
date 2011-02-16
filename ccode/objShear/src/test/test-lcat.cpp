#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include "../lcat.h"


int main(int argc, char** argv) {

    if (argc != 2 ) {
        printf("usage: test-lcat filename\n");
        exit(45);
    }

    const char* filename = argv[1];

    std::vector<struct lens> lcat;

    read_lens(filename, lcat);

    print_lens_firstlast(lcat);

    add_lens_distances(100.0, 0.3, lcat);

    print_lens_firstlast(lcat);
}

