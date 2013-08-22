#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "randn.h"
#include "shape.h"
#include "dist.h"
#include "mtx2.h"

int main(int argc, char **argv)
{
    struct dist_g_ba dist={0};
    struct shape shape={0}, shear={0};
    double P=0, Q1=0,Q2=0, R11=0,R12=0,R22=0;

    struct vec2 Qsum={0}, QbyP={0}, mean_shear={0};
    struct mtx2 QQ={0}, Cinv={0}, Cinv_sum={0},C={0};

    double sigma=0.3;


    if (argc < 5) {
        fprintf(stderr,"usage: test-pqr seed shear1 shear2 num\n");
        exit(1);
    }

    const char *seed_str=argv[1];
    double shear1=atof(argv[2]);
    double shear2=atof(argv[3]);
    long num=atol(argv[4]);

    shape_set_g(&shear, shear1, shear2);
    init_genrand_str(seed_str);

    dist_g_ba_fill(&dist, sigma);

    for (long i=0; i<num; i++) {
        dist_g_ba_sample(&dist, &shape);

        shape_add_inplace(&shape, &shear);

        dist_g_ba_pqr(&dist,
                      &shape,
                      &P,
                      &Q1,
                      &Q2,
                      &R11,
                      &R12,
                      &R22);

        if (P > 0) {
            mtx2_set(&QQ, Q1*Q1, Q1*Q2, Q2*Q2);

            double Pinv = 1.0/P;
            double P2inv = Pinv*Pinv;

            mtx2_set(&Cinv,
                     Q1*Q1*P2inv - R11*Pinv,
                     Q1*Q2*P2inv - R12*Pinv,
                     Q2*Q2*P2inv - R22*Pinv);

            vec2_set(&QbyP, Q1*Pinv, Q2*Pinv);

            vec2_sumi(&Qsum, &QbyP);
            mtx2_sumi(&Cinv_sum, &Cinv);

        }

    }

    mtx2_invert(&Cinv_sum, &C);
    mtx2_vec2prod(&C, &Qsum, &mean_shear);

    printf("shear:   %.16g +/- %.16g  %.16g +/- %.16g\n",
           mean_shear.v1, sqrt(C.m11),
           mean_shear.v2, sqrt(C.m22));

    printf("fracerr: %.16g +/- %.16g  %.16g +/- %.16g\n",
           mean_shear.v1/shear1-1, sqrt(C.m11)/shear1,
           mean_shear.v2/shear2-1, sqrt(C.m22)/shear2);


}
