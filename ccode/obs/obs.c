#include <stdlib.h>
#include <stdio.h>

#include "obs.h"


void obs_fill(struct obs *self,
              const struct image *image,
              const struct image *weight,
              const struct jacobian *jacob,
              const struct gmix *psf,
              long *flags)
{

    self->image=image_free(self->image);
    self->weight=image_free(self->image);
    self->psf=gmix_free(self->psf);

    // need error checking
    self->image=image_newcopy(image);
    self->weight=image_newcopy(weight);

    // jacob is easy straight copy since it is
    // a value type
    self->jacob=(*jacob);
    self->psf=gmix_newcopy(psf,flags);
    if (*flags) {
        goto _obs_fill_bail;
    }

_obs_fill_bail:
    if (*flags) {
        // we will let owner clean up; they must anyway
        //self->image=image_free(self->image);
        //self->weight=image_free(self->image);
        //self->psf=gmix_free(self->psf);
        ;
    }
}

struct obs *obs_new(const struct image *image,
                    const struct image *weight,
                    const struct jacobian *jacob,
                    const struct gmix *psf,
                    long *flags)
{
    struct obs *self=calloc(1,sizeof(struct obs));
    if (!self) {
        fprintf(stderr,"could not allocate struct obs\n");
        exit(1);
    }

    obs_fill(self, image, weight, jacob, psf, flags);
    if (*flags) {
        self=obs_free(self);
    }
    return self;
}


struct obs *obs_free(struct obs *self)
{
    if (self) {
        self->image = image_free(self->image);
        self->weight = image_free(self->weight);
        self->psf = gmix_free(self->psf);
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
        struct obs *obs=NULL;
        size_t i=0;
        for (i=0; i<self->size; i++) {
            obs=&self->data[i];
            obs->image = image_free(obs->image);
            obs->weight = image_free(obs->weight);
            obs->psf = gmix_free(obs->psf);
        }
        free(self->data);
        self=NULL;
    }
    return self;
}
