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

    shear_calc(shear);

    // print some summary info to the terminal
    shear_print_sum(shear);

#ifdef NO_INCREMENTAL_WRITE
    shear_write(shear);
#endif

    shear->fptr = shear_close_tempfile(shear);
    shear_copy_temp_to_output(shear);
    shear_cleanup_tempfile(shear);

    shear_delete(shear);

    printf("Done\n");

    return 0;
}
