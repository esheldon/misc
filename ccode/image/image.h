/*
  This is basically modeled after PHOTO regions
 */
#ifndef _IMAGE_HEADER_GUARD_H
#define _IMAGE_HEADER_GUARD_H

#include <stdlib.h>
#include <stdio.h>
#include "bound.h"

// nrows,ncols,size represent the visible portion, which
// can be a masked subset
struct image {
    size_t size;   // masked size
    size_t nrows;  // masked nrows
    size_t ncols;  // masked ncols
    size_t row0;   // corner of mask
    size_t col0;  
    double counts; // counts of region in mask

    size_t _size;   // true size
    size_t _nrows;  // true nrows
    size_t _ncols;  // true ncols
    double _counts; // total counts

    double sky;
    double skysig;

    int is_owner;
    double **rows;
};



#define IM_SIZE(im) ((im)->size)
#define IM_NROWS(im) ((im)->nrows)
#define IM_NCOLS(im) ((im)->ncols)
#define IM_SKY(im) ( (im)->sky )
#define IM_COUNTS(im) ( (im)->counts )
#define IM_SET_SKY(im, val) ( (im)->sky = (val) )

#define IM_IS_OWNER(im) ( (im)->is_owner )

// counts will be updated consistently in most cases, so
// this is usually not needed except maybe when using
// image_from_array, etc.
#define IM_SET_COUNTS(im, val) ( (im)->counts = (val) )

#define IM_HAS_MASK(im)                              \
    ( (im)->row0 != 0                                \
      || (im)->col0 != 0                             \
      || (im)->nrows != (im)->_nrows                 \
      || (im)->ncols != (im)->_ncols )

#define IM_UNMASK(im) do {                                                   \
    (im)->row0=0;                                                            \
    (im)->col0=0;                                                            \
    (im)->size=(im)->_size;                                                  \
    (im)->nrows=(im)->_nrows;                                                \
    (im)->ncols=(im)->_ncols;                                                \
    (im)->counts=(im)->_counts;                                              \
} while(0)



#define IM_PARENT_SIZE(im) ((im)->_size)
#define IM_PARENT_NROWS(im) ((im)->_nrows)
#define IM_PARENT_NCOLS(im) ((im)->_ncols)
#define IM_PARENT_COUNTS(im) ( (im)->_counts )

#define IM_ROW0(im) ((im)->row0)
#define IM_COL0(im) ((im)->col0)

#define IM_ROW(im,row) \
    ((im)->rows[(im)->row0 + (row)] + (im)->col0)
#define IM_ROW_ITER(im,row) \
    ((im)->rows[(im)->row0 + (row)] + (im)->col0)
#define IM_ROW_END(im,row) \
    ((im)->rows[(im)->row0 + (row)] + (im)->col0 + (im)->ncols)

#define IM_GET(im, row, col)                  \
    ( *((im)->rows[(im)->row0 + (row)] + (im)->col0 + col) )

#define IM_SETFAST(im, row, col, val)                  \
    ( *((im)->rows[(im)->row0 + (row)] + (im)->col0 + col) = (val) )

#define IM_GETP(im, row, col)                 \
    (  ((im)->rows[(im)->row0 + (row)] + (im)->col0 + col) )
/*
 
   Safe way to set pixels, keeping the counts consistent.  If you are doing an
   update of lots of pixels, better to work at a lower level as this is rather
   slow.

*/
#define IM_SET(im, row, col, val) do {                                       \
    double* ptr=IM_GETP(im,row,col);                                         \
    (im)->_counts += (val) - (*ptr);                                         \
    (im)->counts  += (val) - (*ptr);                                         \
    (*ptr) = (val);                                                          \
} while (0)



struct image *image_new(size_t nrows, size_t ncols);
struct image *_image_new(size_t nrows, size_t ncols, int alloc_data);

// If image is masked, only the region inside the mask is copied
struct image *image_copy(const struct image *image);

// in this case we own the rows only, not the data to which they point
struct image* image_from_array(double* data, size_t nrows, size_t ncols);
// get a new image that just references the data in another image.
// in this case we own the rows only, not the data to which they point
// good for applying masks 
struct image* image_getref(const struct image* image);
struct image *image_read_text(const char* filename);

struct image *image_free(struct image *self);

// note the bounds will be trimmed to within the image
void image_add_mask(struct image *self, const struct bound* bound, int update_counts);

void image_write(const struct image *self, FILE* stream);

void image_calc_counts(struct image *self);

// add a scalar to the image, within the mask. Keep the counts
// consistent
void image_add_scalar(struct image *self, double val);

int image_compare(const struct image *im1, const struct image *im2,
                   double *meandiff, double *std);

#endif
