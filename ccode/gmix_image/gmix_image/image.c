#include <stdlib.h>
#include <stdio.h>
#include "image.h"
#include "defs.h"

struct image *image_new(size_t nrows, size_t ncols)
{
    struct image *self=NULL;
    size_t nel = nrows*ncols;
    if (nel == 0) {
        wlog("image size must be > 0\n");
        return NULL;
    }

    self = calloc(1, sizeof(struct image));
    if (self==NULL) {
        wlog("could not allocate struct image\n");
        return NULL;
    }

    self->nrows=nrows;
    self->ncols=ncols;
    self->size=nel;

    self->data = calloc(self->size, sizeof(double));
    if (self->data==NULL) {
        wlog("could not allocate image of dimensions [%lu,%lu]\n",nrows,ncols);
        free(self);
        return NULL;
    }

    return self;

}

struct image *image_free(struct image *self)
{
    if (self) {
        free(self->data);
        self->data=NULL;
        free(self);
        self=NULL;
    }
    return self;
}

struct image *image_read_text(const char* filename)
{
    FILE* fobj=fopen(filename,"r");
    if (fobj==NULL) {
        wlog("Could not open file for reading: %s\n", filename);
        return NULL;
    }

    double sky=0;
    size_t nrows, ncols;
    if (2 != fscanf(fobj, "%lu %lu", &nrows, &ncols)) {
        wlog("Could not read nrows ncols from header\n");
        return NULL;
    }
    if (1 != fscanf(fobj, "%lf", &sky)) {
        wlog("Could not read sky from header\n");
        return NULL;
    }
    struct image* image = image_new(nrows, ncols);

    image_set_sky(image, sky);

    size_t nel=IMSIZE(image);
    double *ptr=IMGETP(image, 0, 0);
    double counts=0;
    while (nel > 0) {
        if (1 != fscanf(fobj, "%lf", ptr)) {
            size_t ntot=IMSIZE(image);
            wlog("Could not read element %lu from file %s\n",
                 ntot-nel, filename);
            image=image_free(image);
            return NULL;
        }
        counts += *ptr;
        ptr++;
        nel--;
    }

    image->has_counts=1;
    image->counts=counts;

    return image;
}

void image_set_sky(struct image *self, double sky)
{
    self->has_sky=1;
    self->sky=sky;
}
double image_sky(struct image *self)
{
    if (!self->has_sky) {
        wlog("sky is not set\n");
        exit(EXIT_FAILURE);
    }
    return self->sky;
}


void image_calc_counts(struct image *self)
{
    double counts=0;
    double *ptr = IMGETP(self,0,0);
    size_t nel=IMSIZE(self);
    while (nel > 0) {
        counts += *ptr;
        ptr++;
        nel--;
    }
    self->has_counts=1;
    self->counts=counts;
}
double image_counts(struct image *self)
{
    if (!self->has_counts) {
        image_calc_counts(self);
    }

    return self->counts;
}
void image_normalize(struct image *self, double norm)
{
    double counts=image_counts(self);
    double fac = norm/counts;

    double new_counts=0.0;

    double *ptr = IMGETP(self,0,0);
    size_t nel=IMSIZE(self);
    while (nel > 0) {
        *ptr *= fac;
        new_counts += *ptr;
        ptr++;
        nel--;
    }
    self->counts=new_counts;
    if (self->has_sky) {
        self->sky *= fac;
    }
}
