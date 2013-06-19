#ifndef _READ_DATA_HEADER_GUARD
#define _READ_DATA_HEADER_GUARD

#include <fitsio.h>
#include "image.h"


struct image *image_read_fits(const char *fname, int ext);

#endif
