#ifndef _GMIX_SIM1_HEADER_GUARD
#define _GMIX_SIM1_HEADER_GUARD

#include "gmix.h"
#include "image.h"

// images will be 2*sigma*GMIX_SIM1_GAUSS_PADDING
// on each side
#define GMIX_SIM1_NSIGMA 5.0

/*

   A single simulated object

*/
struct gmix_sim1 {
    struct gmix *gmix;

    // the number of "sigma" used to determine the size
    // can be non-integer
    double nsigma;

    // the number of points in each dimension used for the
    // sub-pixel integration
    int nsub;

    // if we have not added noise these will be <= 0
    double s2n;
    double skysig;

    // the image created from the above gmix object
    // this is owned
    struct image *image;
};



/*
   Create a new simulation object for the input gmix

   The center is ignored.  The dimensions of the image
   will be determined internally, as will the center.
*/
struct gmix_sim1 *gmix_sim1_cocen_new(const struct gmix *gmix, int nsub);

struct gmix_sim1 *gmix_sim1_free(struct gmix_sim1 *self);



int gmix_sim1_add_noise(struct gmix_sim1 *self, double s2n);

#endif
