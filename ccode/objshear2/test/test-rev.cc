#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include "../rev.h"


int main(int argc, char** argv) {

    if (argc != 2 ) {
        printf("usage: test-rev filename\n");
        exit(45);
    }

    const char* filename = argv[1];

    struct revhtm rev;

    read_rev(filename, rev);
    print_rev_sample(rev);
}

