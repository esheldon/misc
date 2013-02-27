#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "randn.h"

void do_seed(void) {
    time_t tm;
    (void) time(&tm);
    srand48((long) tm);
}

int main(int argc, char **argv)
{
    if (argc < 3) {
        printf("test-poisson lambda num\n");
        printf("  The results to to stdout\n");
        exit(EXIT_FAILURE);
    }

    double lambda=atof(argv[1]);
    long num=atol(argv[2]);

    do_seed();
    for (long i=0; i<num; i++) {
        long pval = poisson(lambda);
        printf("%ld\n", pval);
    }
}
