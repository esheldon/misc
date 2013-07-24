#include <stdlib.h>
#include <stdio.h>

#include "obs.h"


void obs_fill(struct obs *self,
              const struct image *image,
              const struct image *weight,
              const struct image *psf_image,
              const struct jacobian *jacob,
              long psf_ngauss,
              long *flags)
{

    self->image     = image_free(self->image);
    self->weight    = image_free(self->weight);
    self->psf_image = image_free(self->psf_image);
    self->psf_gmix  = gmix_free(self->psf_gmix);

    self->image     = image_new_copy(image);
    self->weight    = image_new_copy(weight);
    self->psf_image = image_new_copy(psf_image);
    self->psf_gmix  = gmix_new(psf_ngauss,flags);
    self->jacob     = (*jacob);
}

struct obs *obs_new(const struct image *image,
                    const struct image *weight,
                    const struct image *psf_image,
                    const struct jacobian *jacob,
                    long psf_ngauss,
                    long *flags)
{
    struct obs *self=calloc(1,sizeof(struct obs));
    if (!self) {
        fprintf(stderr,"could not allocate struct obs\n");
        exit(1);
    }

    obs_fill(self, image, weight, psf_image, jacob, psf_ngauss, flags);

    if (*flags != 0) {
        self=obs_free(self);
    }

    return self;
}


struct obs *obs_free(struct obs *self)
{
    if (self) {
        self->image     = image_free(self->image);
        self->weight    = image_free(self->weight);
        self->psf_image = image_free(self->psf_image);
        self->psf_gmix  = gmix_free(self->psf_gmix);

        free(self);
        self=NULL;
    }
    return self;
}

struct obs_list *obs_list_new(size_t size)
{
    struct obs_list *self=calloc(1,sizeof(struct obs_list));
    if (!self) {
        fprintf(stderr,"could not allocate struct obs_list\n");
        exit(1);
    }

    self->size=size;
    self->data = calloc(size, sizeof(struct obs));
    if (!self->data) {
        fprintf(stderr,"could not allocate %lu struct obs\n",size);
        exit(1);
    }
    return self;
}


struct obs_list *obs_list_free(struct obs_list *self)
{
    if (self) {
        for (long i=0; i<self->size; i++) {
            struct obs *obs=&self->data[i];
            obs->image     = image_free(obs->image);
            obs->weight    = image_free(obs->weight);
            obs->psf_image = image_free(obs->psf_image);
            obs->psf_gmix  = gmix_free(obs->psf_gmix);
        }

        free(self->data);
        free(self);
        self=NULL;
    }
    return self;
}
