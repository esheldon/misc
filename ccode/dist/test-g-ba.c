#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "randn.h"
#include "dist.h"
#include "shape.h"

int main(int argc, char **argv)
{
    if (argc < 3) {
        printf("test-g-ba seed nrand\n");
        exit(1);
    }

    const char *seed_str=argv[1];
    long nrand=atol(argv[2]);

    init_genrand_str(seed_str);

    struct dist_g_ba dist={0};
    double sigma=0.3;
    dist_g_ba_fill(&dist, sigma);


    struct shape shape={0}, shear={0};

    shape_set_g(&shape, 0.2, 0.1);
    shape_set_g(&shear, 0.04, 0.0);

    long flags=0;
    double pj = dist_g_ba_pj(&dist, &shape, &shear,&flags);
    double P,Q1,Q2,R11,R12,R22;
    dist_g_ba_pqr(&dist,
                  &shape,
                  &P,
                  &Q1,
                  &Q2,
                  &R11,
                  &R12,
                  &R22);



    shape_show(&shape, stderr);
    shape_show(&shear, stderr);

    fprintf(stderr,"PQR\n");
    fprintf(stderr,"pj: %.16g\n", pj);
    fprintf(stderr,"P:  %.16g\n", P);
    fprintf(stderr,"Q1:  %.16g\n", Q1);
    fprintf(stderr,"Q2:  %.16g\n", Q2);

    fprintf(stderr,"R11:  %.16g\n", R11);
    fprintf(stderr,"R12:  %.16g\n", R12);
    fprintf(stderr,"R22:  %.16g\n", R22);

    dist_g_ba_pqr_num(&dist,
                  &shape,
                  &P,
                  &Q1,
                  &Q2,
                  &R11,
                  &R12,
                  &R22,
                  &flags);




    fprintf(stderr,"PQR numerical\n");
    fprintf(stderr,"pj: %.16g\n", pj);
    fprintf(stderr,"P:  %.16g\n", P);
    fprintf(stderr,"Q1:  %.16g\n", Q1);
    fprintf(stderr,"Q2:  %.16g\n", Q2);

    fprintf(stderr,"R11:  %.16g\n", R11);
    fprintf(stderr,"R12:  %.16g\n", R12);
    fprintf(stderr,"R22:  %.16g\n", R22);


    shape_show(&shape, stderr);
    shape_show(&shear, stderr);


    for (long i=0; i<nrand; i++) {
        dist_g_ba_sample(&dist, &shape);
        double lnp = dist_g_ba_lnprob(&dist, &shape);
        //double p = dist_g_ba_prob(&dist, &shape);

        dist_g_ba_pqr(&dist,
                      &shape,
                      &P,
                      &Q1,
                      &Q2,
                      &R11,
                      &R12,
                      &R22);


        printf("%.16g %.16g %.16g %.16g %.16g %.16g %.16g %.16g %.16g\n", 
               shape.g1, shape.g2, lnp, 
               P, Q1, Q2, R11, R12, R22);
    }

}
