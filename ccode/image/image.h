/*
  This is basically modeled after PHOTO regions
 */
#ifndef _IMAGE_HEADER_GUARD_H
#define _IMAGE_HEADER_GUARD_H

#include <stdlib.h>
#include <stdio.h>

// nrows,ncols,size represent the visible portion, which
// can be a masked subset
struct image {
    size_t size;   // masked size
    size_t nrows;  // masked nrows
    size_t ncols;  // masked ncols
    size_t row0;   // corner of mask
    size_t col0;  

    size_t _size;   // true size
    size_t _nrows;  // true nrows
    size_t _ncols;  // true ncols

    double sky;
    double skysig;

    int is_owner;
    double **rows;
};



#define IM_SIZE(im) ((im)->size)
#define IM_NROWS(im) ((im)->nrows)
#define IM_NCOLS(im) ((im)->ncols)
#define IM_SKY(im) ( (im)->sky )
#define IM_SET_SKY(im, val) ( (im)->sky = (val) )

#define IM_IS_OWNER(im) ( (im)->is_owner )

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
} while(0)



#define IM_PARENT_SIZE(im) ((im)->_size)
#define IM_PARENT_NROWS(im) ((im)->_nrows)
#define IM_PARENT_NCOLS(im) ((im)->_ncols)

#define IM_ROW0(im) ((im)->row0)
#define IM_COL0(im) ((im)->col0)

#define IM_ROW(im,row) \
    ((im)->rows[(im)->row0 + (row)] + (im)->col0)
#define IM_ROW_ITER(im,row) \
    ((im)->rows[(im)->row0 + (row)] + (im)->col0)
#define IM_ROW_END(im,row) \
    ((im)->rows[(im)->row0 + (row)] + (im)->col0 + (im)->ncols)

#define IM_GET(im, row, col)                  \
    ( *((im)->rows[(im)->row0 + (row)] + (im)->col0 + (col)) )

#define IM_SETFAST(im, row, col, val)                  \
    ( *((im)->rows[(im)->row0 + (row)] + (im)->col0 + (col)) = (val) )

// in the future this could become bounds checking
#define IM_SET IM_SETFAST

#define IM_GETP(im, row, col)                 \
    (  ((im)->rows[(im)->row0 + (row)] + (im)->col0 + (col)) )


// note masks can have negative indices; the image code should deal with it
// properly

struct image_mask {
    ssize_t rowmin;
    ssize_t rowmax;
    ssize_t colmin;
    ssize_t colmax;
};



struct image *image_new(size_t nrows, size_t ncols);
struct image *_image_new(size_t nrows, size_t ncols, int alloc_data);

// If image is masked, only the region inside the mask is copied
// if the images are not the same shape, then 0 is returned and
// no copy is made.  Otherwise 1 is returned
int image_copy(const struct image *image, struct image *imout);

// make a new copy, conforming to the region in the mask
struct image *image_new_copy(const struct image *image);

// in this case we own the rows only, not the data to which they point
struct image* image_from_array(double* data, size_t nrows, size_t ncols);

// get a new image that just references the data in another image.
// in this case we own the rows only, not the data to which they point
// good for applying masks 
struct image* image_get_ref(const struct image* image);

struct image *image_read(const char* filename);

struct image *image_free(struct image *self);

double image_get_counts(const struct image *self);
void image_get_minmax(const struct image *self, double *min, double *max);

// fix the bounds in one dimension so that the range lies
// within [0,dim)
void image_fix_mask(size_t dim, ssize_t *minval, ssize_t *maxval);

// note the masks will be trimmed to within the image
void image_add_mask(struct image *self, const struct image_mask* mask);

void image_write(const struct image *self, FILE* stream);
int image_write_file(const struct image *self, const char *fname);

// add a scalar to the image, within the mask. Keep the counts
// consistent
void image_add_scalar(struct image *self, double val);
void image_set_scalar(struct image *self, double val);

// get the mean difference and variance between two images.
// returns 0 if the images are not the same shape, otherwise 1
int image_compare(const struct image *im1, const struct image *im2,
                   double *meandiff, double *var);




struct image_mask *image_mask_new(ssize_t rowmin, 
                                  ssize_t rowmax, 
                                  ssize_t colmin, 
                                  ssize_t colmax);
struct image_mask *image_mask_free(struct image_mask *self);

void image_mask_set(struct image_mask* self,
                    ssize_t rowmin, 
                    ssize_t rowmax, 
                    ssize_t colmin, 
                    ssize_t colmax);

void image_mask_print(const struct image_mask *mask, FILE *stream);


void image_view(const struct image *self, const char *options);


#endif
