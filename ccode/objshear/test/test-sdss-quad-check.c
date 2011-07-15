#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "../defs.h"
#include "../sdss-survey.h"

/*
 * Read output from make-quad-input, calculate the quadrant, and then
 * calculate if this object is in a "good" quadrant given the input
 * maskflags
 *
 * We do expect a few "wrong" quadrant calculations at the edges.
 *
 */

int main(int argc, char** argv) {

    int quadrant;
    double cenra=0,cendec=0;
    double rra=0, rdec=0;

    double cen_sinlam=0, cen_coslam=0, cen_sineta=0, cen_coseta=0;
    double r_sinlam=0, r_coslam=0, r_sineta=0, r_coseta=0;

    if (argc < 2) {
        printf("test-sdss-quad-check cen_maskflags\n");
        printf("Enter the maskflags for central point\n");
        exit(45);
    }
    int maskflags = atoi(argv[1]);

    if (3 != fscanf(stdin,"%lf %lf %d", &cenra, &cendec, &quadrant)) {
        printf("Expected cenra cendec quadrant at start\n");
        exit(45);
    }
    printf("\ncenter: %lf %lf\n", cenra, cendec);
    printf("expected quadrant: %d\n", quadrant);

    printf("\n");

    eq2sdss_sincos(cenra,cendec,
                   &cen_sinlam, &cen_coslam,
                   &cen_sineta, &cen_coseta);

    int64 ntot=0;
    int64 ngood=0;
    while (2==fscanf(stdin,"%lf %lf",&rra, &rdec)) {

        eq2sdss_sincos(rra,rdec,
                       &r_sinlam, &r_coslam,
                       &r_sineta, &r_coseta);

        int good = test_quad_sincos(maskflags,
                                    cen_sinlam, cen_coslam,
                                    cen_sineta, cen_coseta,
                                    r_sinlam, r_coslam,
                                    r_sineta, r_coseta);



        if (good) {
            ngood++;
        }

        ntot++;
    }

    printf("%ld/%ld pass maskflags\n", ngood, ntot);

}
