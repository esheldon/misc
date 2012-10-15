#ifndef _GMIX_IMAGE_HEADER_GUARD
#define _GMIX_IMAGE_HEADER_GUARD

#define GMIX_IMAGE_NEGATIVE_DET 0x1

#include <stdlib.h>
#include "gmix.h"

struct image *gmix_image_new(struct gmix *gmix, 
                             size_t nrows, 
                             size_t ncols, 
                             int nsub);
int gmix_image_fill(struct image *image, 
                    struct gmix *gmix, 
                    int nsub);

#endif
