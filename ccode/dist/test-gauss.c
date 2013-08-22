#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "randn.h"
#include "dist.h"

int main(int argc, char **argv)
{
    if (argc < 5) {
        printf("test-gauss seed mean sigma nrand\n");
        exit(1);
    }

    const char *seed_str=argv[1];
    init_genrand_str(seed_str);

    double mean=atof(argv[2]);
    double sigma=atof(argv[3]);
    long nrand=atol(argv[4]);

    struct dist_gauss dist={0};
    dist_gauss_fill(&dist, mean, sigma);

    for (long i=0; i<nrand; i++) {
        double val = dist_gauss_sample(&dist);
        printf("%.16g\n", val);
    }
}
