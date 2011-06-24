#include <stdlib.h>
#include <stdio.h>
#include "config.h"
#include "lens.h"
#include "source.h"


int main(int argc, char** argv) {
    if (argc < 2) {
        printf("usage: objshear config_file\n");
        exit(EXIT_FAILURE);
    }

    const char* config_file=argv[1];

    struct config* config=read_config(config_file);
    print_config(config);

    struct lcat* lcat = lcat_read(config->lens_file);
    lcat_print_firstlast(lcat);
    struct scat* scat = scat_read(config->source_file);
    scat_print_firstlast(scat);
}
