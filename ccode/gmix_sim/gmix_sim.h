#ifndef _GMIX_SIM_HEADER_GUARD
#define _GMIX_SIM_HEADER_GUARD

#import "gmix.h"
#import "image.h"

// images will be 2*sigma*GMIX_SIM_GAUSS_PADDING
// on each side
#define GMIX_SIM_NSIGMA 5.0

/*

   A single simulated object

*/
struct gmix_sim {
    // the gmix is not owned by this structure, will be an input
    // from the user
    const struct gmix *gmix;

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

struct gmix_noisy_sim {
    const struct gmix_sim *sim;
    double skysig;
    double s2n;
    struct image *image;
};


/*
   Create a new simulation object for the input gmix
*/
struct gmix_sim *gmix_sim_new(const struct gmix *gmix, int nsub);

struct gmix_sim *gmix_sim_del(struct gmix *self);

#endif
