#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "image.h"
#include "bound.h"

struct image *image_new(size_t nrows, size_t ncols)
{
    int do_alloc_data=1;
    struct image *self=NULL;
    self = _image_new(nrows, ncols, do_alloc_data);
    return self;
}

struct image *_image_new(size_t nrows, size_t ncols, int alloc_data)
{
    struct image *self=NULL;
    size_t nel = nrows*ncols, i=0;
    if (nel == 0) {
        fprintf(stderr,"image size must be > 0\n");
        exit(EXIT_FAILURE);
    }

    self = calloc(1, sizeof(struct image));
    if (self==NULL) {
        fprintf(stderr,"could not allocate struct image\n");
        exit(EXIT_FAILURE);
    }

    // These are set forever
    self->_nrows=nrows;
    self->_ncols=ncols;
    self->_size=nel;

    // no mask for now, but these visible sizes can change
    self->nrows=nrows;
    self->ncols=ncols;
    self->size=nel;

    self->rows = calloc(nrows,sizeof(double *));
    if (self->rows==NULL) {
        fprintf(stderr,"could not allocate image of dimensions [%lu,%lu]\n",
                nrows,ncols);
        exit(EXIT_FAILURE);
    }
    if (alloc_data) {
        self->rows[0] = calloc(self->size,sizeof(double));
        if (self->rows[0]==NULL) {
            fprintf(stderr,"could not allocate image of dimensions [%lu,%lu]\n",
                    nrows,ncols);
            exit(EXIT_FAILURE);
        }

        for(i = 1; i < nrows; i++) {
            self->rows[i] = self->rows[i-1] + ncols;
        }
        self->is_owner=1;
    } else {
        self->rows[0] = NULL;
    }


    return self;
}



struct image *image_free(struct image *self)
{
    if (self) {
        if (self->rows) {
            if (self->rows[0] && IM_IS_OWNER(self)) {
                free(self->rows[0]);
            }
            self->rows[0]=NULL;

            free(self->rows);
            self->rows=NULL;
        }
        free(self);
        self=NULL;
    }
    return self;
}

struct image *image_read_text(const char* filename)
{
    FILE* fobj=fopen(filename,"r");
    if (fobj==NULL) {
        fprintf(stderr,"Could not open file for reading: %s\n", filename);
        return NULL;
    }

    double sky=0;
    size_t nrows, ncols;
    if (2 != fscanf(fobj, "%lu %lu", &nrows, &ncols)) {
        fprintf(stderr,"Could not read nrows ncols from header\n");
        return NULL;
    }
    if (1 != fscanf(fobj, "%lf", &sky)) {
        fprintf(stderr,"Could not read sky from header\n");
        return NULL;
    }
    struct image* image = image_new(nrows, ncols);

    IM_SET_SKY(image, sky);

    size_t row=0, col=0;
    double *ptr=NULL;
    double counts=0;
    for (row=0; row<nrows; row++) {
        for (col=0; col<ncols; col++) {

            ptr = IM_GETP(image,row,col);

            if (1 != fscanf(fobj, "%lf", ptr)) {
                fprintf(stderr,"Could not read element (%lu,%lu) from file %s\n",
                        row, col, filename);
                image=image_free(image);
                return NULL;
            }

            counts += (*ptr);
        }
    }

    IM_SET_COUNTS(image, counts);
    return image;
}




void image_write(const struct image *self, FILE* stream)
{
    size_t row=0;
    double *col=NULL, *end=NULL;
    fprintf(stream,"%lu\n", IM_NROWS(self));
    fprintf(stream,"%lu\n", IM_NCOLS(self));
    fprintf(stream,"%.16g\n", IM_SKY(self));

    for (row=0; row<IM_NROWS(self); row++) {
        col = IM_ROW_ITER(self,row);
        end = IM_ROW_END(self,row);
        for (; col != end; col++) {
            fprintf(stream,"%.16g ",*col);
        }
        fprintf(stream,"\n");
    }
}

// bound is gauranteed to be within [0,size).  Also maxval is
// gauranteed to be >= minval.
void fix_bounds(size_t dim, ssize_t *minval, ssize_t *maxval)
{
    if (*minval < 0) {
        *minval=0;
    }
    if (*maxval < 0) {
        *maxval=0;
    }

    if (*minval > (dim-1)) {
        *minval=(dim-1);
    }
    if (*maxval > (dim-1)) {
        *maxval=(dim-1);
    }

    if (*maxval < *minval) {
        *maxval = *minval;
    }
}
void image_add_mask(struct image *self, const struct bound* bound)
{
    ssize_t tminval=0, tmaxval=0;

    tminval=bound->rowmin;
    tmaxval=bound->rowmax;

    fix_bounds(IM_PARENT_NROWS(self), &tminval, &tmaxval);
    self->row0  = (size_t) tminval;
    self->nrows = (size_t) (tmaxval - tminval + 1);

    tminval=bound->colmin;
    tmaxval=bound->colmax;
    fix_bounds(IM_PARENT_NCOLS(self), &tminval, &tmaxval);
    self->col0  = (size_t) tminval;
    self->ncols = (size_t)(tmaxval - tminval + 1);

    self->size = self->nrows*self->ncols;

    // we keep the counts for the sub-image region
    // the parent counts are still available in _counts
    image_calc_counts(self);
}

struct image *image_copy(const struct image *image)
{
    struct image *imcopy=NULL;
    size_t nrows=0, ncols=0, row=0;
    const double *rowdata=NULL;
    double *rowdata_copy=NULL;

    nrows=IM_NROWS(image);
    ncols=IM_NCOLS(image);
    imcopy=image_new(nrows,ncols);

    // could be masked, so do a loop
    for (row=0; row<nrows; row++) {
        rowdata=IM_ROW(image, row);
        rowdata_copy=IM_ROW(imcopy, row);

        memcpy(rowdata_copy, rowdata, ncols*sizeof(double));
    }

    imcopy->counts=image->counts;
    imcopy->_counts=image->_counts;
    imcopy->sky=image->sky;
    imcopy->skysig=image->skysig;

    return imcopy;
}

// in this case we own the rows only, not the data to which they point
struct image* image_from_array(double* data, size_t nrows, size_t ncols)
{
    int dont_alloc_data=0;
    size_t i=0;
    struct image *self=NULL;

    self = _image_new(nrows, ncols, dont_alloc_data);

    self->rows[0] = data;
    for(i = 1; i < nrows; i++) {
        self->rows[i] = self->rows[i-1] + ncols;
    }

    image_calc_counts(self);
    return self;
}

// get a new image that just references the data in another image.
// in this case we own the rows only, not the data to which they point
// good for applying masks 
struct image* image_getref(const struct image* image)
{
    size_t i=0;
    struct image *self=NULL;
    size_t nrows=0, ncols=0;

    nrows=IM_NROWS(image);
    ncols=IM_NCOLS(image);
    self = _image_new(nrows, ncols, 0);

    self->rows[0] = image->rows[0];
    for(i = 1; i < IM_NROWS(self); i++) {
        self->rows[i] = self->rows[i-1] + ncols;
    }

    IM_SET_COUNTS(self, IM_COUNTS(image));
    return self;
}


void image_calc_counts(struct image *self)
{
    double counts=0;
    double *col=NULL, *end=NULL;
    size_t nrows = IM_NROWS(self);
    size_t row=0;
    for (row=0;row<nrows;row++) {

        col = IM_ROW_ITER(self,row);
        end = IM_ROW_END(self,row);
        for (; col != end; col++) {
            counts += *col;
        }
    }
    self->counts=counts;

    if (!IM_HAS_MASK(self)) {
        self->_counts=counts;
    }
}

void image_add_scalar(struct image *self, double val)
{
    double *col=NULL, *end=NULL;
    size_t row=0, nrows = IM_NROWS(self);
    for (row=0;row<nrows;row++) {
        col    = IM_ROW_ITER(self,row);
        end = IM_ROW_END(self,row);
        for (; col != end; col++) {
            self->counts  += val-(*col);
            self->_counts += val-(*col);
            *col += val;
        }
    }

    self->sky += val;
}


int image_compare(const struct image *im1, const struct image *im2,
                   double *meandiff, double *var)
{

    size_t row=0, nrows=0, col=0, ncols=0;
    double *rows1=NULL, *rows2=NULL;
    double diff=0, sum=0, sum2=0;

    (*meandiff)=9999.e9;
    (*var)=9999.e9;

    nrows=IM_NROWS(im1);
    ncols=IM_NCOLS(im1);
    if (nrows != IM_NROWS(im2) || ncols != IM_NCOLS(im2) ) {
        return 0;
    }

    for (row=0; row<nrows; row++) {
        rows1=IM_ROW(im1,row);
        rows2=IM_ROW(im2,row);
        for (col=0; col<ncols; col++) {
            diff = (*rows2) - (*rows1);
            sum += diff;
            sum2 += diff*diff;

            rows1++;
            rows2++;
        }
    }

    (*meandiff) = sum/IM_SIZE(im1);
    (*var) = sum2/IM_SIZE(im1) - (*meandiff)*(*meandiff);

    return 1;
}
