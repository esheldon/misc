#include <stdlib.h>
#include <stdio.h>
#include "gmix.h"
#include "gsim_ring.h"
#include "shape.h"
#include "time.h"

int main(int argc, char **argv)
{
    double pars[5]={0}, psf_pars[6] = {0};
    long npars=5,npars_psf=6,flags=0;
    enum gmix_model model=GMIX_BD;
    enum gmix_model psf_model=GMIX_COELLIP;
    struct gmix *gmix_psf=NULL;
    struct ring_pair *rpair=NULL;
    double s2n=100;
    struct shape shear={0};

    time_t t1;
  
    (void) time(&t1);
    srand48((long) t1);

    shape_set_g(&shear, 0.01, 0.0);

    pars[0] = 0.2;   // eta
    pars[1] = 16.0;  // Tbulge
    pars[2] = 20.0;  // Tdisk
    pars[3] = 0.7;   // Fbulge
    pars[4] = 0.3;   // Fdisk

    psf_pars[0] = -1;
    psf_pars[1] = -1;
    psf_pars[2] = 0.1;
    psf_pars[3] = 0.3;
    psf_pars[4] = 4.0;
    psf_pars[5] = 1.0;

    gmix_psf = gmix_new_model(psf_model,psf_pars,npars_psf,&flags);
    if (flags != 0) {
        goto _bail;
    }

    rpair = ring_pair_new(model, s2n, pars, npars, gmix_psf, &shear, &flags);
    if (flags != 0) {
        goto _bail;
    }

    ring_pair_print(rpair,stdout);

_bail:
    gmix_psf=gmix_free(gmix_psf);
    rpair = ring_pair_free(rpair);

    return 0;
}
