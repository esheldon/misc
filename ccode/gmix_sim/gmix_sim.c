#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "gmix.h"
#include "gmix_image.h"
#include "gmix_sim.h"


/*
   Use [size,size] for dimensions of your image
*/
static int get_imsize(const struct gmix *gmix, double nsigma)
{
    double T=gmix_get_T(gmix);
    double sigma = sqrt(T/2.);

    int imsize = int(round(2*nsigma*sigma));
    if (imsize < 10) {
        imsize=10;
    }
    return imsize;
}
struct gmix_sim *gmix_sim_new(const struct gmix *gmix, int nsub) 
{
    struct gmix_sim *self=NULL;

    self=calloc(1, sizeof(struct gmix_sim));
    if (self==NULL) {
        fprintf(stderr,"could not allocate gmix_sim object\n");
        exit(EXIT_FAILURE);
    }

    int imsize=get_imsize(gmix);

    self->image=gmix_image_new(gmix,imsize,imsize,nsub);
    self->gmix=gmix;
    self->nsigma=GMIX_SIM_NSIGMA;
    self->nsub=nsub;

    self->s2n=-9999;
    self->skysig=-9999;

    return self;
}

struct gmix_sim *gmix_sim_del(struct gmix *self)
{
    if (self) {
        self->image=image_del(self->image);
        free(self);
        self=NULL;
    }
    return self;
}


