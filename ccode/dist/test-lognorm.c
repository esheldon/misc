#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "randn.h"
#include "dist.h"

int main(int argc, char **argv)
{
    if (argc < 4) {
        printf("test-lognorm mean sigma nrand\n");
        exit(1);
    }

    randn_seed();

    double mean=atof(argv[1]);
    double sigma=atof(argv[2]);
    long nrand=atol(argv[3]);

    struct dist_lognorm dist={0};
    dist_lognorm_fill(&dist, mean, sigma);

    for (long i=0; i<nrand; i++) {
        double val = dist_lognorm_sample(&dist);
        printf("%.16g\n", val);
    }
}
