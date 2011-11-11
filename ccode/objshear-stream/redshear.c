#include <stdlib.h>
#include <stdio.h>
#include "lensum.h"
#include "config.h"
#include "log.h"
#include "defs.h"
#include "urls.h"

void usage_and_exit(void) {
    printf("usage: redshear [config_file]\n");
    printf("  If config_file is not sent as an argument, the CONFIG_FILE env variable is used\n");
    exit(EXIT_FAILURE);
}

int64 get_nlens(const char* filename) {
    int64 nlens=0;
    wlog("getting nlens from url: %s\n", filename);
    FILE* stream = open_url(filename,"r");
    if (1 != fscanf(stream, "%ld", &nlens)) {
        wlog("Could not read nlens from file %s\n", filename);
        exit(EXIT_FAILURE);
    }
    fclose(stream);
    return nlens;
}

int main(int argc, char** argv) {
    int64 counter=0;
    const char* config_file=get_config_filename(argc, argv);
    if (config_file==NULL)
        usage_and_exit();

    struct config* config=config_read(config_file);

    int64 nlens = get_nlens(config->lens_file);
    wlog("Found nlens: %ld\n", nlens);
    struct lensums* lensums = lensums_new(nlens, config->nbin);

    struct lensum* lensum = lensum_new(config->nbin);

    while (lensum_read(stdin, lensum)) {
        counter++;
        if (counter == 1) {
            wlog("first lensum: %ld %ld %.8g %ld %.8g\n", 
                 lensum->index, lensum->zindex, lensum->weight, lensum->totpairs, lensum->sshsum);
        }
        if ((counter % 1000) == 0) {
            wlog(".");
        }

        if (lensum->index < 0 || lensum->index >= nlens) {
            wlog("index %ld out of range: [%d,%ld]\n",
                 lensum->index, 0, nlens-1);
            exit(EXIT_FAILURE);
        }

        lensum_add(&lensums->data[lensum->index], lensum);
    }

    wlog("\nlast lensum: %ld %ld %.8g %ld %.8g\n", 
            lensum->index, lensum->zindex, lensum->weight, lensum->totpairs, lensum->sshsum);

    wlog("Read a total of %ld lensums\n", counter);

    // print some summary info
    lensums_print_sum(lensums);

    wlog("Writing results to stdout\n");
    lensums_write(lensums, stdout);

    lensums=lensums_delete(lensums);
    lensum=lensum_delete(lensum);
    wlog("Done\n");

    return 0;
}
