#ifndef _GSIM_RING_HEADER_GUARD
#define _GSIM_RING_HEADER_GUARD

#include "gmix.h"

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

// creates a new ring pair, convolved wih the PSF and sheared
//
// For BD, the pars should be length 5
//     [eta,Tbulge,Tdisk,Fbulge,Fdisk]

struct ring_pair *ring_pair_new(enum gmix_model model,
                                double s2n,
                                const double *pars, long npars,
                                const struct gmix *psf, 
                                double shear1, double shear2,
                                long *flags);

struct ring_pair *ring_pair_free(struct ring_pair *self);

#endif
