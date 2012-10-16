#ifndef _GMIX_IMAGE_HEADER_GUARD
#define _GMIX_IMAGE_HEADER_GUARD

#define GMIX_IMAGE_NEGATIVE_DET 0x1

#include <stdlib.h>
#include "gmix.h"

//#define GMIX_IMAGE_LOW_VAL (-DBL_MAX+1000)
#define GMIX_IMAGE_LOW_VAL (-9999.e9)

/*

   Render the input gaussian mixture model into an image.
   A new image is created and returend.

*/
struct image *gmix_image_new(const struct gmix *gmix, 
                             size_t nrows, 
                             size_t ncols, 
                             int nsub);

/*

   Render the input gaussian mixture model into the
   input image.

*/

int gmix_image_fill(struct image *image, 
                    const struct gmix *gmix, 
                    int nsub);

/*

   calculate the ln(like) between the image and the input gaussian mixture

*/
double gmix_image_loglike(const struct image *image, 
                          const struct gmix *gmix, 
                          double ivar,
                          int *flags);

#endif
