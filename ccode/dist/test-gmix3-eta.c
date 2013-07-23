#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "randn.h"
#include "dist.h"

int main(int argc, char **argv)
{
    if (argc < 2) {
        printf("test-gmix3-eta nrand\n");
        exit(1);
    }

    randn_seed();

    double sigma1=0.22;
    double p1=0.47;

    double sigma2=0.65;
    double p2=0.52;

    double sigma3=1.47;
    double p3=0.006;

    long nrand=atol(argv[1]);

    struct dist_gmix3_eta dist={0};
    dist_gmix3_eta_fill(&dist,
                        sigma1, sigma2, sigma3,
                        p1, p2, p3);

    struct shape shape={0};
    for (long i=0; i<nrand; i++) {
        dist_gmix3_eta_sample(&dist, &shape);
        printf("%.16g %.16g\n", shape.eta1, shape.eta2);
    }
}
