#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "randn.h"

static void do_seed(const char *seed_str)
{
    if (!init_genrand_str(seed_str)) {
        fprintf(stderr,"failed to convert to seed: '%s'\n", seed_str);
        exit(1);
    }
}

int main(int argc, char **argv)
{
    if (argc < 4) {
        printf("gen-poisson seed lambda num\n");
        printf("  The results to to stdout\n");
        exit(1);
    }

    do_seed(argv[1]);

    double lambda=atof(argv[2]);
    long num=atol(argv[3]);

    for (long i=0; i<num; i++) {
        long pval = poisson(lambda);
        printf("%ld\n", pval);
    }
}
