#ifndef _OBS_HEADER_GUARD
#define _OBS_HEADER_GUARD

#include <stddef.h>
#include "image.h"
#include "jacobian.h"
#include "gmix.h"

struct obs {
    struct image *image;
    struct image *weight;
    struct image *psf_image;
    struct jacobian jacob;
    struct gmix *psf_gmix;
};

struct obs_list {
    size_t size;
    struct obs *data;
};

// everything is copied
struct obs *obs_new(const struct image *image,
                    const struct image *weight,
                    const struct image *psf_image,
                    const struct jacobian *jacob,
                    long psf_ngauss,
                    long *flags);

// everything is copied
void obs_fill(struct obs *self,
              const struct image *image,
              const struct image *weight,
              const struct image *psf_image,
              const struct jacobian *jacob,
              long psf_ngauss,
              long *flags);

struct obs *obs_free(struct obs *self);

struct obs_list *obs_list_new(size_t num);
struct obs_list *obs_list_free(struct obs_list *self);

#endif
