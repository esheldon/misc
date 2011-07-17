#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "../defs.h"
#include "../sdss-survey.h"

/*
 * Read ra,dec pairs from standard input and write out ra,dec,lam,eta
 *
 * If an argument is sent, this makes sure the sin() and cos() values are
 * consistent
 */

int main(int argc, char** argv) {

    int do_sincos=0;
    if (argc > 1) {
        do_sincos = 1;
    }

    double ra=0,dec=0;
    double lam,eta;
    double sinlam=0,coslam=0,sineta=0,coseta=0;

    while (2==fscanf(stdin,"%lf %lf",&ra, &dec)) {


        eq2sdss(ra,dec,&lam,&eta);
        if (do_sincos) {
            eq2sdss_sincos(ra,dec,
                           &sinlam, &coslam,
                           &sineta, &coseta);
            
            printf("%.15f %.15f %.15f %.15f\n", 
                   sinlam - sin(lam*D2R), 
                   coslam - cos(lam*D2R),
                   sineta - sin(eta*D2R), 
                   coseta - cos(eta*D2R));
        } else {
            printf("%.15f %.15f %.15f %.15f\n", ra, dec, lam,eta);
        }

    }

}
