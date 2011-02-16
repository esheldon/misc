#include <stdlib.h>
#include <stdio.h>
#include "../Cosmology.h"

#include <time.h>
 
int main(int argc, char** argv) {

    int dospeed=0;
    if (argc > 1) {
        dospeed=1;
    }

    float omega_m = 0.3;
    float H0 = 100.0;

    Cosmology cosmo(H0, omega_m);

    printf("omega_m: %f\n", omega_m);
    printf("H0: %f\n", H0);

    float zmin=0.2, zmax=0.4;
    float ezint = cosmo.Ez_inverse_integral(zmin, zmax);
    printf("ezint(%f, %f): = %f\n", zmin, zmax, ezint);

    float da = cosmo.Da(zmin, zmax);
    printf("Da(%f, %f): = %f\n", zmin, zmax, da);

    float scinv = cosmo.sigmacritinv(zmin, zmax);
    printf("scinv(%f, %f): = %e\n", zmin, zmax, scinv);

    float dl = cosmo.Da(0.0, zmin);
    float ds = cosmo.Da(0.0, zmax);

    scinv = cosmo.sigmacritinv(dl, ds, zmin, zmax);
    printf("scinv(%f, %f, %f, %f): = %e\n", dl, ds, zmin, zmax, scinv);


    if (dospeed) {

        clock_t start = clock();
        int ntest = 1000000;
        printf("ntrials %d\n", ntest);

        for (int i=0; i<ntest; i++) {
            scinv = cosmo.sigmacritinv(zmin, zmax);
        }
        printf("    time just z inputs: %f\n", 
               ((double)clock() - start) / CLOCKS_PER_SEC);


        start = clock();

        for (int i=0; i<ntest; i++) {
            scinv = cosmo.sigmacritinv(dl, ds, zmin, zmax);
        }
        printf("    time precompute dl/ds: %f\n", 
               ((double)clock() - start) / CLOCKS_PER_SEC);

    }
}
