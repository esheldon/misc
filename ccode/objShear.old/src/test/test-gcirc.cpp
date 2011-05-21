#include "../gcirc.h"
#include "../lens_constants.h"
#include <stdio.h>

int main(int argc, char** argv) {
    double ra1 = 200.0;
    double dec1 = 0.0;
    //double ra2 = 201.0;
    //double dec2 = 1.0;
    double ra2 = 201.0;
    double dec2 = 1.0;


    double dis, theta;

    gcirc_eq(ra1, dec1, ra2, dec2, dis, theta);

    printf("ra1: %15.8lf  dec1: %15.8lf\n", ra1, dec1);
    printf("ra2: %15.8lf  dec2: %15.8lf\n", ra2, dec2);

    printf("\ngcirc_eq\n    dis: %15.8lf\n", dis);
    printf("    theta: %15.8lf\n", theta*R2D);

    gcirc(ra1, dec1, ra2, dec2, dis, theta);
    printf("\ngcirc\n    dis: %15.8lf\n", dis);
    printf("    theta: %15.8lf\n", theta*R2D);


    double lam1 = 0.0;
    double eta1 = 0.0;

    double lam2 = 1.0;
    double eta2 = 1.0;


    gcirc_survey(lam1, eta1, lam2, eta2, dis, theta);

    printf("\nlam1: %15.8lf  eta1: %15.8lf\n", lam1, eta1);
    printf("lam2: %15.8lf  eta2: %15.8lf\n", lam2, eta2);

    printf("\ndis: %15.8lf\n", dis);
    printf("theta: %15.8lf\n", theta*R2D);

}

