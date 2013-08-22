#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "randn.h"
#include "dist.h"

int main(int argc, char **argv)
{
    if (argc < 5) {
        printf("test-lognorm seed mean sigma nrand\n");
        exit(1);
    }

    const char *seed_str=argv[1];
    init_genrand_str(seed_str);

    double mean=atof(argv[2]);
    double sigma=atof(argv[3]);
    long nrand=atol(argv[4]);

    struct dist_lognorm dist={0};
    dist_lognorm_fill(&dist, mean, sigma);

    for (long i=0; i<nrand; i++) {
        double val = dist_lognorm_sample(&dist);
        double lnp = dist_lognorm_lnprob(&dist, val);
        printf("%.16g %.16g\n", val, lnp);
    }
}
