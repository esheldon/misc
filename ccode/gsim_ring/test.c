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
    double pars[5]={0}, psf_pars[6] = {0};
    long npars=5,npars_psf=6,flags=0;
    enum gmix_model model=GMIX_BD;
    enum gmix_model psf_model=GMIX_COELLIP;
    struct gmix *gmix_psf=NULL;
    struct ring_pair *rpair=NULL;
    double s2n=1000;
    struct shape shear={0};
    struct image *im1=NULL;
    struct image *im2=NULL;
    double skysig1=0,skysig2=0;
    double cen1_offset=0.1, cen2_offset=-0.2;
    int nsub=16;

    time_t t1;
  
    (void) time(&t1);
    srand48((long) t1);

    shape_set_g(&shear, 0.01, 0.0);

    pars[0] = 0.4;   // eta
    pars[1] = 20.0;  // Tbulge
    pars[2] = 100.0;  // Tdisk
    pars[3] = 20;   // Fbulge
    pars[4] = 80;   // Fdisk

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

    rpair = ring_pair_new(model, pars, npars, gmix_psf, &shear, s2n, &flags);
    if (flags != 0) {
        goto _bail;
    }

    ring_pair_print(rpair,stdout);

    im1 = ring_make_image(rpair->gmix1,
                          cen1_offset,
                          cen2_offset,
                          nsub,
                          rpair->s2n,
                          &skysig1,
                          &flags);

    if (flags != 0) {
        goto _bail;
    }
    im2 = ring_make_image(rpair->gmix2,
                          cen1_offset,
                          cen2_offset,
                          nsub,
                          rpair->s2n,
                          &skysig2,
                          &flags);

    if (flags != 0) {
        goto _bail;
    }

    printf("skysig1: %g  skysig2: %g\n", skysig1, skysig2);

    show_image(im1, "/tmp/im1.dat");
    show_image(im2, "/tmp/im2.dat");

_bail:
    gmix_psf=gmix_free(gmix_psf);
    rpair = ring_pair_free(rpair);
    im1=image_free(im1);
    im2=image_free(im2);

    return 0;
}
