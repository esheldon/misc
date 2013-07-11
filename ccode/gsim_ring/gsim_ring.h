#ifndef _GSIM_RING_HEADER_GUARD
#define _GSIM_RING_HEADER_GUARD

#include <stdio.h>
#include "gmix.h"
#include "shape.h"

#define GAUSS_PADDING 5

// for now models are always bulge+disk, so pars are
// 
//    [eta,eta2,Tbulge,Tdisk,Fbulge,Fdisk]
//
// it is OK to have either flux zero.  
//
// The PSF is for now a simple model with only a scale length and shape.  Can
// be GMIX_COELLIP (but only one gauss) or GMIX_TURB.
//
//    [eta1,eta2,T]
//

struct ring_pair {
    double s2n;
    long npars;
    struct gmix *gmix1;
    struct gmix *gmix2;
};

// the shortened pars
long ring_get_npars_short(enum gmix_model model, long *flags);

// creates a new ring pair, convolved wih the PSF and sheared

struct ring_pair *ring_pair_new(enum gmix_model model,
                                const double *pars, long npars,
                                const struct gmix *psf, 
                                const struct shape *shear,
                                double s2n,
                                long *flags);

struct ring_pair *ring_pair_free(struct ring_pair *self);

void ring_pair_print(const struct ring_pair *self, FILE* stream);

struct image *ring_make_image(const struct gmix *gmix,
                              double cen1_offset,
                              double cen2_offset,
                              int nsub,
                              double s2n,
                              double *skysig, // output
                              long *flags);

#endif
