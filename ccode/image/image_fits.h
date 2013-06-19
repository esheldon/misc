#ifndef _IMAGE_FITS_HEADER_GUARD
#define _IMAGE_FITS_HEADER_GUARD

#include "image.h"

/*
   Converts to image type, which is currently always double.
*/
struct image *image_read_fits(const char *fname, int ext);

void image_write_fits(const struct image *self, 
                      const char *filename, 
                      int clobber);

#endif
