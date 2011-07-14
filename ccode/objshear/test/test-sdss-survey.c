#include <stdlib.h>
#include <stdio.h>
#include "../sdss-survey.h"

/*
 * Read ra,dec pairs from standard input and write out
 * ra,dec,lam,eta
 */

int main(int argc, char** argv) {

    double ra=0,dec=0;
    double lam,eta;
    while (2==fscanf(stdin,"%lf %lf",&ra, &dec)) {
        eq2sdss(ra,dec,&lam,&eta);
        printf("%.15f %.15f %.15f %.15f\n", ra, dec, lam,eta);
    }

}
