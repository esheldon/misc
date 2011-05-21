#include <stdlib.h>
#include <stdio.h>
#include "../angDist.h"
#include "../sigmaCritInv.h"
#include "../Cosmology.h"
#include <time.h>

int main(int argc, char** argv) {

    if (argc < 3 ) {
        printf("usage: test-scinv zlens zsource\n");
        printf("  if third arg given, a speed comparison with Cosmology full\n");
        printf("  integral is done\n");
        exit(45);
    }

    int dospeed = 0;
    if (argc > 3) {
        dospeed = 1;
    }

    float zlens = atof(argv[1]);
    float zsource = atof(argv[2]);

    float omega_m = 0.30;
    float H0 = 100.0;

    float DL = angDist(H0, omega_m, zlens);


    float aeta0 = aeta(0,omega_m);
    float aeta_rel_lens   = aeta0 - aeta(zlens, omega_m);
    float aeta_rel_source = aeta0 - aeta(zsource, omega_m);

    float scinv = sigmaCritInv(DL,aeta_rel_lens, aeta_rel_source);

    printf("omega_m: %f\n", omega_m);
    printf("H0: %f\n", H0);
    printf("scinv(%f, %f) = %e (pc^2/Msun)\n", zlens,zsource,scinv);

    Cosmology cosmo(H0, omega_m);
    // use high npts for this
    cosmo.set_weights(1000);
    float scinv_int = cosmo.sigmacritinv(zlens, zsource);
    printf("scinv from full integral = %e\n", scinv_int);
    printf("fracdiff: %f\n", (scinv_int - scinv)/scinv_int);

    if (dospeed) {

        clock_t start;
        int ntest = 50000000;

        printf("Doing speed test comparision with full integral\n");
        printf("ntrials: %d\n", ntest);

        start = clock();
        for (int i=0; i<ntest; i++) {
            scinv = sigmaCritInv(DL,aeta_rel_lens, aeta_rel_source);
        }
        printf("    time approximage formula:\n        %f\n", 
               ((double)clock() - start) / CLOCKS_PER_SEC);



        int npts=3;
        cosmo.set_weights(npts);

        start = clock();
        for (int i=0; i<ntest; i++) {
            scinv = cosmo.sigmacritinv(zlens, zsource);
        }
        printf("    time integral, npts=%d, just z inputs:\n        %f\n", 
               npts, ((double)clock() - start) / CLOCKS_PER_SEC);


        float dl = cosmo.Da(0.0, zlens);
        float ds = cosmo.Da(0.0, zsource);
        start = clock();

        for (int i=0; i<ntest; i++) {
            scinv = cosmo.sigmacritinv(dl, ds, zlens, zsource);
        }
        printf("    time integral, npts=%d, precompute dl/ds:\n        %f\n", 
               npts, ((double)clock() - start) / CLOCKS_PER_SEC);

    }

}
