#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "image.h"

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
        fprintf(stderr,"error: image size must be > 0\n");
        exit(EXIT_FAILURE);
    }

    self = calloc(1, sizeof(struct image));
    if (self==NULL) {
        fprintf(stderr,"error: could not allocate struct image\n");
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

struct image *image_read(const char* filename)
{
    FILE* fobj=fopen(filename,"r");
    if (fobj==NULL) {
        fprintf(stderr,"Could not open file for reading: %s\n", filename);
        return NULL;
    }

    size_t nrows, ncols;
    if (2 != fscanf(fobj, "%lu %lu", &nrows, &ncols)) {
        fprintf(stderr,"Could not read nrows ncols from header\n");
        return NULL;
    }
    struct image* image = image_new(nrows, ncols);

    size_t row=0, col=0;
    double *ptr=NULL;
    for (row=0; row<nrows; row++) {
        for (col=0; col<ncols; col++) {

            ptr = IM_GETP(image,row,col);

            if (1 != fscanf(fobj, "%lf", ptr)) {
                fprintf(stderr,"Could not read element (%lu,%lu) from file %s\n",
                        row, col, filename);
                image=image_free(image);
                return NULL;
            }
        }
    }

    return image;
}


int image_write_file(const struct image *self, const char *fname)
{
    FILE *fobj=fopen(fname,"w");
    if (fobj==NULL) {
        fprintf(stderr,"Could not open file '%s'\n", fname);
        return 0;
    }

    image_write(self, fobj);
    fclose(fobj);

    return 1;
}



void image_write(const struct image *self, FILE* stream)
{
    size_t row=0;
    double *col=NULL, *end=NULL;
    fprintf(stream,"%lu %lu\n", IM_NROWS(self), IM_NCOLS(self));

    for (row=0; row<IM_NROWS(self); row++) {
        col = IM_ROW_ITER(self,row);
        end = IM_ROW_END(self,row);
        for (; col != end; col++) {
            fprintf(stream,"%.16g ",*col);
        }
        fprintf(stream,"\n");
    }
}

// mask is gauranteed to be within [0,size).  Also maxval is
// gauranteed to be >= minval.
void image_fix_mask(size_t dim, ssize_t *minval, ssize_t *maxval)
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
void image_add_mask(struct image *self, 
                    const struct image_mask* mask)
{
    ssize_t tminval=0, tmaxval=0;

    tminval=mask->rowmin;
    tmaxval=mask->rowmax;

    image_fix_mask(IM_PARENT_NROWS(self), &tminval, &tmaxval);
    self->row0  = (size_t) tminval;
    self->nrows = (size_t) (tmaxval - tminval + 1);

    tminval=mask->colmin;
    tmaxval=mask->colmax;
    image_fix_mask(IM_PARENT_NCOLS(self), &tminval, &tmaxval);
    self->col0  = (size_t) tminval;
    self->ncols = (size_t)(tmaxval - tminval + 1);

    self->size = self->nrows*self->ncols;

}

int image_copy(const struct image *image, struct image *imout)
{
    size_t nrows=0, ncols=0, row=0;
    double *rowdata=NULL, *rowdata_out;

    nrows=IM_NROWS(image);
    ncols=IM_NCOLS(image);
    if (nrows != IM_NROWS(imout) 
            || ncols != IM_NCOLS(imout)) {
        return 0;
    }
    // could be masked, so do a loop
    for (row=0; row<nrows; row++) {
        rowdata=IM_ROW(image, row);
        rowdata_out=IM_ROW(imout, row);

        memcpy(rowdata_out, rowdata, ncols*sizeof(double));
    }
    return 1;
}
struct image *image_new_copy(const struct image *image)
{
    struct image *imout=NULL;
    size_t nrows=0, ncols=0, row=0;
    const double *rowdata=NULL;
    double *rowdata_out=NULL;

    nrows=IM_NROWS(image);
    ncols=IM_NCOLS(image);
    imout=image_new(nrows,ncols);

    // could be masked, so do a loop
    for (row=0; row<nrows; row++) {
        rowdata=IM_ROW(image, row);
        rowdata_out=IM_ROW(imout, row);

        memcpy(rowdata_out, rowdata, ncols*sizeof(double));
    }

    imout->sky=image->sky;
    imout->skysig=image->skysig;

    return imout;
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

    return self;
}

// get a new image that just references the data in another image.
// in this case we own the rows only, not the data to which they point
// good for applying masks 

struct image* image_get_ref(const struct image* image)
{
    size_t nrows=IM_PARENT_NROWS(image);
    size_t ncols=IM_PARENT_NCOLS(image);

    struct image *self=calloc(1, sizeof(struct image));
    if (!self) {
        fprintf(stderr,"error: could not allocate struct image\n");
        exit(EXIT_FAILURE);
    }

    // copy over the metadata
    (*self) = (*image);
    self->is_owner = 0;

    // we want our own version of the rows pointers
    self->rows = calloc(nrows,sizeof(double *));
    if (self->rows==NULL) {
        fprintf(stderr,"could not rows of dimensions %lu\n",
                nrows);
        exit(EXIT_FAILURE);
    }

    self->rows[0] = image->rows[0];
    for(size_t i = 1; i < nrows; i++) {
        self->rows[i] = self->rows[i-1] + ncols;
    }

    return self;
}

double image_get_counts(const struct image *self)
{
    double counts=0;
    double *col=NULL, *end=NULL;
    size_t nrows = IM_NROWS(self);
    size_t row=0;
    for (row=0;row<nrows;row++) {

        col = IM_ROW_ITER(self,row);
        end = IM_ROW_END(self,row);
        for (; col != end; col++) {
            counts += (*col);
        }
    }
    return counts;
}
void image_get_minmax(const struct image *self, double *min, double *max)
{
    *min=0;
    *max=0;

    double *data=IM_ROW(self, 0);
    size_t size=IM_SIZE(self);

    *min=*data;
    *max=*data;
    for (size_t i=0; i<size; i++) {
        if (*data > *max) {
            *max=*data;
        }
        if (*data < *min) {
            *min=*data;
        }
        data++;
    }
}


void image_add_scalar(struct image *self, double val)
{
    double *col=NULL, *end=NULL;
    size_t row=0, nrows = IM_NROWS(self);
    for (row=0;row<nrows;row++) {
        col = IM_ROW_ITER(self,row);
        end = IM_ROW_END(self,row);
        for (; col != end; col++) {
            (*col) += val;
        }
    }

    self->sky += val;
}
void image_set_scalar(struct image *self, double val)
{
    double *col=NULL, *end=NULL;
    size_t row=0, nrows = IM_NROWS(self);
    for (row=0;row<nrows;row++) {
        col = IM_ROW_ITER(self,row);
        end = IM_ROW_END(self,row);
        for (; col != end; col++) {
            (*col) = val;
        }
    }

    self->sky = val;
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


struct image_mask *image_mask_new(ssize_t rowmin, 
                        ssize_t rowmax, 
                        ssize_t colmin, 
                        ssize_t colmax)
{
    struct image_mask *self=NULL;

    self = calloc(1, sizeof(struct image_mask));
    if (self==NULL) {
        fprintf(stderr,"could not allocate struct image_mask\n");
        exit(EXIT_FAILURE);
    }

    image_mask_set(self, rowmin, rowmax, colmin, colmax);
    return self;
}

struct image_mask *image_mask_free(struct image_mask *self) {
    if (self) {
        free(self);
    }
    return NULL;
}

void image_mask_set(struct image_mask* self,
                     ssize_t rowmin, 
                     ssize_t rowmax, 
                     ssize_t colmin, 
                     ssize_t colmax)
{
    self->rowmin=rowmin;
    self->rowmax=rowmax;
    self->colmin=colmin;
    self->colmax=colmax;
}

void image_mask_print(const struct image_mask *mask, FILE *stream)
{
    fprintf(stream,"  rowmin: %ld\n", mask->rowmin);
    fprintf(stream,"  rowmax: %ld\n", mask->rowmax);
    fprintf(stream,"  colmin: %ld\n", mask->colmin);
    fprintf(stream,"  colmin: %ld\n", mask->colmax);
}


void image_view(const struct image *self, const char *options)
{
    char cmd[256];
    char *name= tempnam(NULL,NULL);
    printf("writing temporary image to: %s\n", name);
    FILE *fobj=fopen(name,"w");
    int ret=0;
    image_write(self, fobj);

    fclose(fobj);

    sprintf(cmd,"image-view %s %s", options, name);
    printf("%s\n",cmd);
    ret=system(cmd);

    sprintf(cmd,"rm %s", name);
    printf("%s\n",cmd);
    ret=system(cmd);
    printf("ret: %d\n", ret);

    free(name);
}

