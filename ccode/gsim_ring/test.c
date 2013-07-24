#include <stdlib.h>
#include <stdio.h>
#include "gmix.h"
#include "gsim_ring.h"
#include "shape.h"
#include "image.h"
#include "time.h"

void show_image(const struct image *self, const char *name)
{
    char cmd[256];
    printf("writing temporary image to: %s\n", name);
    FILE *fobj=fopen(name,"w");
    int ret=0;
    image_write(self, fobj);

    fclose(fobj);

    sprintf(cmd,"image-view -m %s", name);
    printf("%s\n",cmd);
    ret=system(cmd);

    sprintf(cmd,"rm %s", name);
    printf("%s\n",cmd);
    ret=system(cmd);
    printf("ret: %d\n", ret);
}
int main(int argc, char **argv)
{
    double pars[5]={0}, psf_pars[3] = {0};
    long npars=5,psf_npars=3,flags=0;
    enum gmix_model model=GMIX_BD;
    enum gmix_model psf_model=GMIX_COELLIP;
    struct gmix *gmix_psf=NULL;
    struct ring_pair *rpair=NULL;
    struct ring_image_pair *impair=NULL;
    double s2n=100;
    struct shape shear={0};
    double cen1_offset=0.1, cen2_offset=-0.2;

    time_t t1;
  
    (void) time(&t1);
    srand48((long) t1);

    shape_set_g(&shear, 0.01, 0.0);

    pars[0] = 0.4;   // eta
    pars[1] = 20.0;  // Tbulge
    pars[2] = 100.0;  // Tdisk
    pars[3] = 20;   // Fbulge
    pars[4] = 80;   // Fdisk

    psf_pars[0] = 0.1;
    psf_pars[1] = 0.3;
    psf_pars[2] = 4.0;

    rpair = ring_pair_new(model, pars, npars, psf_model, psf_pars, psf_npars, &shear, s2n, 
                          cen1_offset, cen2_offset, &flags);
    if (flags != 0) {
        goto _bail;
    }

    ring_pair_print(rpair,stdout);

    impair=ring_image_pair_new(rpair, &flags);
    if (flags != 0) {
        goto _bail;
    }

    printf("skysig1: %g  skysig2: %g\n", impair->skysig1, impair->skysig2);

    show_image(impair->im1, "/tmp/im1.dat");
    show_image(impair->im2, "/tmp/im2.dat");

_bail:
    gmix_psf=gmix_free(gmix_psf);
    rpair = ring_pair_free(rpair);
    impair = ring_image_pair_free(impair);

    return 0;
}
