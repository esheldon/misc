/*
   Tools for adding noise to images.  Only normal random deviates so far.
*/
#ifndef _IMAGE_RAND_HEADER_GUARD
#define _IMAGE_RAND_HEADER_GUARD

#include "image.h"

/*
   Add normal random deviates with sigma "skysig"
*/
void image_add_randn(struct image *image, double skysig);

void image_add_randn_matched(struct image *image, double s2n, double *skysig);

/*
   replace the image values with a poisson deviate
*/
void image_add_poisson(struct image *image);

#endif
