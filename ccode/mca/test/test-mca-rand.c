#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "../mca_rand.h"


int main(int argc, char **argv)
{
    size_t ntrial=1000000;
    long nmax=10;
    time_t tm;

    (void) time(&tm);
    srand48((long) tm);

    if (argc > 1) {
        ntrial=atol(argv[1]);
    }
    // first test the basic random number generator
    for (size_t i=0; i<ntrial; i++) {
        long r = mca_randlong(nmax);
        if (r < 0 || r >= nmax) {
            fprintf(stderr,
                    "Error, found rand outside of range: [0,%ld)\n", nmax);
            exit(EXIT_FAILURE);
        }
    }

    // now as a complement
    long current=3;
    for (size_t i=0; i<ntrial; i++) {
        long r = mca_rand_complement(current, nmax);
        if (r==current) {
            fprintf(stderr,
                    "Error, rand equal to the complement: [0,%ld)\n", current);
            exit(EXIT_FAILURE);
        }
    }

    printf("all tests passed\n");

}
