#ifndef _GMIX_IMAGE_HEADER_GUARD
#define _GMIX_IMAGE_HEADER_GUARD

#define GMIX_IMAGE_NEGATIVE_DET 0x1

#include <stdlib.h>
#include "image.h"
#include "gmix.h"

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
double gmix_image_loglike(const struct image *image, 
                          const struct gmix *gmix, 
                          double ivar,
                          int *flags);


double gmix_image_s2n(const struct image *image, 
                      double skysig, 
                      const struct gmix *weight,
                      int *flags);

int gmix_image_add_noise(struct image *image, 
                         double s2n,
                         const struct gmix *gmix,
                         double *skysig);

#endif
