#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "gmix.h"
#include "gmix_image.h"
#include "gmix_image_rand.h"
#include "gmix_sim1.h"


/*
   Use [size,size] for dimensions of your image
*/
static int get_imsize(const struct gmix *gmix, double nsigma)
{

    double T=gmix_get_T(gmix);
    double sigma = sqrt(T/2.);
    int imsize = (int)(round(2*nsigma*sigma));
    return imsize;
}
struct gmix_sim1 *gmix_sim1_cocen_new(const struct gmix *gmix, int nsub) 
{
    long flags=0;
    struct gmix_sim1 *self=NULL;

    self=calloc(1, sizeof(struct gmix_sim1));
    if (self==NULL) {
        fprintf(stderr,"could not allocate gmix_sim1 object\n");
        exit(EXIT_FAILURE);
    }

    self->nsigma=GMIX_SIM1_NSIGMA;
    int imsize=get_imsize(gmix,self->nsigma);
    double cen=(imsize-1)/2.;

    // whoops, ignoring flags
    self->gmix=gmix_new_copy(gmix,&flags);
    for (size_t i=0; i<gmix->size; i++) {
        self->gmix->data[i].row=cen;
        self->gmix->data[i].col=cen;
    }

    self->image=gmix_image_new(self->gmix,imsize,imsize,nsub);
    self->nsub=nsub;

    self->s2n=-9999;
    self->skysig=-9999;

    return self;
}

struct gmix_sim1 *gmix_sim1_free(struct gmix_sim1 *self)
{
    if (self) {
        self->image=image_free(self->image);
        self->gmix=gmix_free(self->gmix);
        free(self);
        self=NULL;
    }
    return self;
}


int gmix_sim1_add_noise(struct gmix_sim1 *self, double s2n)
{

    int flags=
        gmix_image_add_randn(self->image, 
                             s2n,
                             self->gmix,
                             &self->skysig);
#if 0
    int flags2=0;
    double s2n_meas=gmix_image_s2n(self->image,
                                   self->skysig,
                                   self->gmix,
                                   &flags2);
    fprintf(stderr,"measured s2n: %.16g\n", s2n_meas);
#endif
    if (flags != 0) {
        fprintf(stderr,"error adding noise to image: %d\n", flags);
        return flags;
    }

    self->s2n=s2n;

    return 0;
}
