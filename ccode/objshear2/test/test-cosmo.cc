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
    float dls,dcl,dcs,scinv,scinvpre;

    printf("omega_m: %f\n", omega_m);
    printf("H0: %f\n", H0);

    Cosmology cosmo(H0, omega_m);
    printf("Using npts=%d\n", cosmo.npts);

    float zlens=0.2, zsource=0.4;

    dcl = cosmo.dc(0.0, zlens);
    dcs = cosmo.dc(0.0, zsource);

    float ezint = cosmo.ez_inverse_integral(zlens, zsource);
    printf("    ezint(%f, %f): = %f\n", zlens, zsource, ezint);

    printf("\n");
    scinv = cosmo.scinv(zlens, zsource);
    scinvpre = cosmo.scinv(zlens, dcl, dcs);

    printf("    scinv(%f, %f): = %e\n", zlens, zsource, scinv);
    printf("    precompute: %e\n", scinvpre);
    printf("    precompute perc diff: %f\n", 100*(scinv-scinvpre)/scinv);


    if (dospeed) {

        clock_t start;
        double tm, tmpre;

        int ntest = 10000000;
        printf("ntrials %d\n", ntest);


        start = clock();
        for (int i=0; i<ntest; i++) {
            dcs = cosmo.dc(0.0, zsource);
        }
        tm = ((double)clock() - start) / CLOCKS_PER_SEC;
        printf("    dc time: %f\n", tm);


        start = clock();

        for (int i=0; i<ntest; i++) {
            dcs = cosmo.dc_approx(0.0, zsource);
        }

        tmpre = ((double)clock() - start) / CLOCKS_PER_SEC;
        printf("    dctime approx: %f\n", tmpre);

        printf("speedup: %f\n", tm/tmpre);


        printf("\n\n");


        float zlensarr[] = {0.2,0.21,0.19};

        start = clock();
        for (int i=0; i<ntest; i++) {
            int ii = i % 3;
            zlens = zlensarr[ii];
            scinv = cosmo.scinv(zlens, zsource);
        }
        tm = ((double)clock() - start) / CLOCKS_PER_SEC;
        printf("    time just z inputs: %f\n", tm);


        start = clock();

        // note with -O2 g++ is too smart, it will
        // run this only once sinc dcl/dcs are constants!
        for (int i=0; i<ntest; i++) {
            int ii = i % 3;
            zlens = zlensarr[ii];
            scinv = cosmo.scinv(zlens, dcl, dcs);
        }

        tmpre = ((double)clock() - start) / CLOCKS_PER_SEC;
        printf("    time precompute: %f\n", tmpre);

        printf("speedup: %f\n", tm/tmpre);

    }
}
