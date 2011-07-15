#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "../defs.h"
#include "../sdss-survey.h"

/*
 * Read output from make-quad-input, calculate the quadrant, and make sure
 * it agrees with expected.
 *
 * We do expect a few "wrong" quadrant calculations at the edges.
 *
 */

int main(int argc, char** argv) {

    int quadrant;
    double cenra=0,cendec=0;
    double cenlam=0,ceneta=0;
    double rra=0, rdec=0;
    double rlam=0, reta=0;

    double cen_sinlam=0, cen_coslam=0, cen_sineta=0, cen_coseta=0;
    double r_sinlam=0, r_coslam=0, r_sineta=0, r_coseta=0;

    double dis=0,theta=0,theta_deg=0;


    if (3 != fscanf(stdin,"%lf %lf %d", &cenra, &cendec, &quadrant)) {
        printf("Expected cenra cendec quadrant at start\n");
        exit(45);
    }
    printf("\ncenter: %lf %lf\n", cenra, cendec);
    printf("expected quadrant: %d\n", quadrant);

    int do_sincos=0;
    if (argc > 1) {
        do_sincos=1;
        //printf("Using fast sin/cos math\n");
    }
    printf("\n");

    eq2sdss(cenra, cendec, &cenlam, &ceneta);

    int64 ntot=0;
    int64 nbad=0;
    while (2==fscanf(stdin,"%lf %lf",&rra, &rdec)) {

        eq2sdss(rra,rdec,&rlam,&reta);
        if (do_sincos) {
            if (ntot == 0) printf("Using fast sin/cos math\n\n");
            eq2sdss_sincos(cenra,cendec,
                           &cen_sinlam, &cen_coslam,
                           &cen_sineta, &cen_coseta);
            eq2sdss_sincos(rra,rdec,
                           &r_sinlam, &r_coslam,
                           &r_sineta, &r_coseta);
            theta = posangle_survey_sincos(cen_sinlam, cen_coslam,
                                           cen_sineta, cen_coseta,
                                           r_sinlam, r_coslam,
                                           r_sineta, r_coseta);
        } else {
            gcirc_survey(cenlam, ceneta, rlam, reta, &dis, &theta);
        }

        int rquad = survey_quad(theta);
                      
        if (rquad != quadrant) {
            // this is the angle used for quadrant determination
            theta_deg = (M_PI_2 - theta)*R2D;
            printf("mismatch: %.15f %.15f %.15f %d\n", rra, rdec, theta_deg, rquad);
            nbad++;
        }

        ntot++;
    }

    printf("%ld/%ld mismatches\n", nbad, ntot);

}
