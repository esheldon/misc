#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "randn.h"
#include "shape.h"
#include "dist.h"
#include "mtx2.h"


static int do_set(struct vec2 *QbyP,
                  struct mtx2 *Cinv,
                  double P, double Q1, double Q2,
                  double R11, double R12, double R22)
{
    struct mtx2 QQ={0};
    if (P > 0) {
        mtx2_set(&QQ, Q1*Q1, Q1*Q2, Q2*Q2);

        double Pinv = 1.0/P;
        double P2inv = Pinv*Pinv;

        mtx2_set(Cinv,
                 Q1*Q1*P2inv - R11*Pinv,
                 Q1*Q2*P2inv - R12*Pinv,
                 Q2*Q2*P2inv - R22*Pinv);

        vec2_set(QbyP, Q1*Pinv, Q2*Pinv);


        return 1;
    } else {
        return 0;
    }

}
static int do_pqr_pair(const struct dist_g_ba *dist,
                       const struct shape *shape1,
                       const struct shape *shape2,
                       struct vec2 *Qsum,
                       struct mtx2 *Cinv_sum)
{

    double P=0, Q1=0,Q2=0, R11=0,R12=0,R22=0;
    struct vec2 QbyP_1={0}, QbyP_2={0};
    struct mtx2 Cinv_1={0}, Cinv_2={0};

    dist_g_ba_pqr(dist,
                  shape1,
                  &P,
                  &Q1,
                  &Q2,
                  &R11,
                  &R12,
                  &R22);

    if (!do_set(&QbyP_1, &Cinv_1, P, Q1, Q2, R11, R12, R22)) {
        return 0;
    }
    dist_g_ba_pqr(dist,
                  shape2,
                  &P,
                  &Q1,
                  &Q2,
                  &R11,
                  &R12,
                  &R22);

    if (!do_set(&QbyP_2, &Cinv_2, P, Q1, Q2, R11, R12, R22)) {
        return 0;
    }

    vec2_sumi(Qsum, &QbyP_1);
    mtx2_sumi(Cinv_sum, &Cinv_1);
    vec2_sumi(Qsum, &QbyP_2);
    mtx2_sumi(Cinv_sum, &Cinv_2);

    return 1;
}

int main(int argc, char **argv)
{
    struct dist_g_ba dist={0};
    struct shape shape1={0}, shape2={0}, shear={0};

    struct vec2 Qsum={0}, mean_shear={0};
    struct mtx2 Cinv_sum={0},C={0};

    double sigma=0.3;


    if (argc < 5) {
        fprintf(stderr,"usage: test-pqr seed shear1 shear2 npair\n");
        exit(1);
    }

    const char *seed_str=argv[1];
    double shear1=atof(argv[2]);
    double shear2=atof(argv[3]);
    long npair=atol(argv[4]);

    shape_set_g(&shear, shear1, shear2);
    init_genrand_str(seed_str);

    dist_g_ba_fill(&dist, sigma);

    for (long i=0; i<npair; i++) {
        //if ( (i % 1000) == 0) {
        //    printf("%ld/%ld\n", i+1, npair);
        //}
        while (1) {
            dist_g_ba_sample(&dist, &shape1);
            shape2 = shape1;
            shape_rotate(&shape2, M_PI_2);

            shape_add_inplace(&shape1, &shear);
            shape_add_inplace(&shape2, &shear);

            if (do_pqr_pair(&dist,
                            &shape1,
                            &shape2,
                            &Qsum,
                            &Cinv_sum)) {
                break;
            }
        }
    }

    mtx2_invert(&Cinv_sum, &C);
    mtx2_vec2prod(&C, &Qsum, &mean_shear);

    printf("%.16g %.16g %.16g   %.16g %.16g %.16g\n",
           shear1, mean_shear.v1/shear1-1, sqrt(C.m11)/shear1,
           shear2, mean_shear.v2/shear2-1, sqrt(C.m22)/shear2);


}
