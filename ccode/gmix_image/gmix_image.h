#ifndef _GMIX_IMAGE_HEADER_GUARD
#define _GMIX_IMAGE_HEADER_GUARD

#define GMIX_IMAGE_NEGATIVE_DET 0x1

#include <stdlib.h>
#include "image.h"
#include "gmix.h"
#include "jacobian.h"

#define GMIX_IMAGE_LOW_VAL (-9999.e49)
#define GMIX_IMAGE_BIGNUM 9.999e9

/*

   Render the input gaussian mixture model into an image.
   A new image is created and returend.

*/
struct image *gmix_image_new(const struct gmix *gmix, 
                             size_t nrows, 
                             size_t ncols, 
                             int nsub);

/*

   Add the input gaussian mixture model to the input image.
   Make sure your image is initialized beforehand.

*/

int gmix_image_put(struct image *image, 
                   const struct gmix *gmix, 
                   int nsub);

/*
   Add the gaussian mixture only within the region indicated by the mask.
*/
int gmix_image_put_masked(struct image *image, 
                          const struct gmix *gmix, 
                          int nsub,
                          struct image_mask *mask);


/*

   calculate the ln(like) between the image and the input gaussian mixture

*/

// using a weight image and jacobian.
// row0,col0 is center of coordinate system
// gmix centers should be in the u,v plane
// combine s2n_numer and s2n_denom as below
// can sum over multiple images
// s2n = s2n_numer/sqrt(s2n_denom);

int gmix_image_loglike_wt_jacob(const struct image *image, 
                                const struct image *weight,
                                const struct jacobian *jacob,
                                const struct gmix *gmix, 
                                double *s2n_numer,
                                double *s2n_denom,
                                double *loglike);

// weight image but no jacobian
// row0,col0 is center of coordinate system
// gmix centers should be in the u,v plane
// combine s2n_numer and s2n_denom as below
// can sum over multiple images
// s2n = s2n_numer/sqrt(s2n_denom);

int gmix_image_loglike_wt(const struct image *image, 
                          const struct image *weight,
                          const struct gmix *gmix, 
                          double *s2n_numer,
                          double *s2n_denom,
                          double *loglike);

// ivar with jacobian
// using a weight image.  Not tested.
// combine s2n_numer and s2n_denom as below
// can sum over multiple images
// s2n = s2n_numer/sqrt(s2n_denom);

int gmix_image_loglike_ivar_jacob(const struct image *image, 
                                  double ivar,
                                  const struct jacobian *jacob,
                                  const struct gmix *gmix, 
                                  double *s2n_numer,
                                  double *s2n_denom,
                                  double *loglike);

// ivar and no jacobian
// using a weight image.  Not tested.
// combine s2n_numer and s2n_denom as below
// can sum over multiple images
//s2n = s2n_numer/sqrt(s2n_denom);

int gmix_image_loglike_ivar(const struct image *image, 
                            const struct gmix *gmix, 
                            double ivar,
                            double *s2n_numer,
                            double *s2n_denom,
                            double *loglike);


double gmix_image_s2n_ivar(const struct image *image, 
                           const struct gmix *weight,
                           double ivar, 
                           long *flags);
//int gmix_image_add_noise(struct image *image, 
//                         double s2n,
//                         const struct gmix *gmix,
//                         double *skysig);

#endif
