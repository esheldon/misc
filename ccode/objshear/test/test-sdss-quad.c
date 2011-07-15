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

    double dis=0,theta=0,theta_deg=0;

    if (3 != fscanf(stdin,"%lf %lf %d", &cenra, &cendec, &quadrant)) {
        printf("Expected cenra cendec quadrant at start\n");
        exit(45);
    }
    printf("center: %lf %lf\n", cenra, cendec);
    printf("expected quadrant: %d\n", quadrant);

    eq2sdss(cenra, cendec, &cenlam, &ceneta);

    int64 ntot=0;
    int64 nbad=0;
    while (2==fscanf(stdin,"%lf %lf",&rra, &rdec)) {
        ntot++;

        eq2sdss(rra,rdec,&rlam,&reta);

        gcirc_survey(cenlam, ceneta, rlam, reta, &dis, &theta);

        //theta *= R2D;
        theta_deg = (M_PI_2 - theta)*R2D;
        //theta = (theta-M_PI)*R2D;
        
        int rquad = gcirc_theta_quad(theta);
                      
        if (rquad != quadrant) {
            printf("mismatch: %.15f %.15f %.2f %d\n", rra, rdec, theta_deg, rquad);
        }

    }

    printf("%ld/%ld mismatches\n", nbad, ntot);

}
