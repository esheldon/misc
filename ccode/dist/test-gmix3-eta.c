/*
   stdout has some randoms

   on stderr goes a test of pqr
*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "randn.h"
#include "dist.h"

int main(int argc, char **argv)
{
    if (argc < 3) {
        printf("test-gmix3-eta seed nrand\n");
        exit(1);
    }


    const char *seed_str=argv[1];
    long nrand=atol(argv[2]);

    init_genrand_str(seed_str);

    double sigma1=0.22;
    double p1=0.47;

    double sigma2=0.65;
    double p2=0.52;

    double sigma3=1.47;
    double p3=0.006;


    struct dist_gmix3_eta dist={0};
    dist_gmix3_eta_fill(&dist,
                        sigma1, sigma2, sigma3,
                        p1, p2, p3);

    struct shape shape={0};
    struct shape shear={0};
    shape_set_g(&shape, 0.2, 0.1);
    shape_set_g(&shear, 0.04, 0.0);

    long flags=0;
    double pj = dist_gmix3_eta_pj(&dist, &shape, &shear, &flags);
    double P,Q1,Q2,R11,R12,R22;
    dist_gmix3_eta_pqr(&dist,
                  &shape,
                  &P,
                  &Q1,
                  &Q2,
                  &R11,
                  &R12,
                  &R22,
                  &flags);



    shape_show(&shape, stdout);
    shape_show(&shear, stdout);

    fprintf(stderr,"pj: %.16g\n", pj);
    fprintf(stderr,"P:  %.16g\n", P);
    fprintf(stderr,"Q1:  %.16g\n", Q1);
    fprintf(stderr,"Q2:  %.16g\n", Q2);

    fprintf(stderr,"R11:  %.16g\n", R11);
    fprintf(stderr,"R12:  %.16g\n", R12);
    fprintf(stderr,"R22:  %.16g\n", R22);

    for (long i=0; i<nrand; i++) {
        dist_gmix3_eta_sample(&dist, &shape);
        printf("%.16g %.16g\n", shape.eta1, shape.eta2);
    }
}
