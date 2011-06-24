#include <stdlib.h>
#include <stdio.h>
#include "shear.h"

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("usage: objshear config_file\n");
        exit(EXIT_FAILURE);
    }

    const char* config_file=argv[1];

    struct shear* shear = shear_init(config_file);

    //calc_shear(shear);

    shear_delete(shear);

}
