#include <stdlib.h>
#include <stdio.h>
#include "source.h"
#include "shear.h"
#include "log.h"
#include "defs.h"
//#include "shear.h"

void usage_and_exit(void) {
    printf("usage: sobjshear [config_file]\n");
    printf("  If config_file is not sent as an argument, the CONFIG_FILE env variable is used\n");
    exit(EXIT_FAILURE);
}

const char* get_config_filename(int argc, char** argv) {
    const char* config_file=NULL;
    if (argc >= 2) {
        config_file=argv[1];
    } else {
        config_file=getenv("CONFIG_FILE");
        if (config_file==NULL) {
            usage_and_exit();
        }
    }

    return config_file;
}

int main(int argc, char** argv) {
    int64 counter=0;
    const char* config_file=get_config_filename(argc, argv);

    struct shear* shear=shear_init(config_file);

#ifndef WITH_TRUEZ
    struct source* src=source_new(shear->config->nzl);
    src->zlens = shear->config->zl;
#else
    struct source* src=source_new();
#endif

    while (source_read(stdin, src)) {
        counter++;
        if (counter == 1) {
            wlog("first source:\n");
            source_print(src);
        }
        if ((counter % 10000) == 0) {
            wlog(".");
        }

#ifdef WITH_TRUEZ
        src->dc = Dc(shear->cosmo, 0.0, src->z);
#endif

        shear_process_source(shear, src);
    }
    wlog("\nlast source:\n");
    source_print(src);

    wlog("Read a total of %lu sources\n", counter);

    // print some summary info to the terminal
    shear_print_sum(shear);
    //shear_write(shear);

    src=source_delete(src);
    shear=shear_delete(shear);
    wlog("Done\n");

    return 0;
}
