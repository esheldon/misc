#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include "sheardata.h"


int main(int argc, char** argv) {

    if (argc != 2 ) {
        printf("usage: objshear config_file\n");
        exit(45);
    }

    const char* config_file = argv[1];

    struct sheardata data;

    read_sheardata(config_file, data);
    data.output.init(data.pars.nbin);
    data.output.open(data.pars.output_file);

    data.output.close();
}

