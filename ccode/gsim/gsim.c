/*

    gsim config_file catalog image

    for the simulation config, read objects and psf info from the catalog file
    (one per line), place them in an image with noise and write to the image
    file.

    config file
    -----------

    The config file should have at least these entries.  Others
    can exist also and will be ignored.
        nrow = integer
            Number of rows in the image
        ncol = integer
            Number of columns in the image
        noise_type = "string"
            Type of noise in the image.  Currently
            "gauss" or "poisson"

            For poisson noise, a deviate is drawn based on
            the value sky+flux in each pixel.

            For gaussian, sqrt(sky) is used for the width of gaussian noise.

        sky = double
            Value for the sky.  Specify zero for no noise.
        nsub = integer
            the number of points to use for sub-pixel integration over the
            pixels.  16 is a good value, even overkill in most cases.
            
        ellip_type = "string"
            "e" or "g".  if "e" then the ellipticities in the file are given as
            (a^2-b^2)/(a^2+b^2).  If ellip_type is "g" they are (a-b)/(a+b)

        seed = integer
            Seed for the random number generator


    catalog file
    ------------

    The catalog file should be a space-delimited text file with the following
    columns

        model row col e1 e2 T counts psfmodel psfe1 psfe2 psfT

    Note that for gaussians, the centroid and counts are not given

    column description

        - model should currently be 
            "gauss" - a single gaussian
            "exp"   - an exponential represented using gaussians
            "dev"   - an devaucouleur represented using gaussians
            "turb"  - turbulent psf represented using gaussians

          If you want a bulge+disk, just put two entries in the file.  You can
          add any number of components to an object this way.

        - row,col
            zero-offset row and column in the image

        - e1,e2
            The ellipticity of the object or the psf.  The convention should
            match that given by the config file.  You should put
            shear into this value.

        - T is the <x^2> + <y^2> for the object or psf.
   
    output image
    ------------
    The output image is written to the indicated file.  The format
    is FITS.

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gconfig.h"
#include "object.h"
#include "image.h"
#include "gmix.h"
#include "gmix_image.h"
#include "image_rand.h"

#define GMIX_PADDING 5.0

FILE *open_catalog(const char *filename)
{
    fprintf(stderr,"opening catalog: %s\n", filename);
    FILE *catfile = fopen(filename,"r");
    if (catfile==NULL) {
        fprintf(stderr,"failed to open catalog: %s\n", filename);
        exit(EXIT_FAILURE);
    }

    return catfile;

}

struct image *make_image(const struct gconfig *conf)
{
    fprintf(stderr,"making image\n");
    struct image *im=image_new(conf->nrow, conf->ncol);
    fprintf(stderr,"adding sky\n");
    image_add_scalar(im,conf->sky);
    return im;
}

struct gmix *make_object_gmix(struct object *object)
{
    double pars[6] = {};
    pars[0] = object->row;
    pars[1] = object->col;
    pars[2] = object->e1;
    pars[3] = object->e2;
    pars[4] = object->T;
    pars[5] = object->counts;

    struct gmix *gmix=NULL;
    if ( strcmp(object->model, "exp") ) {
        gmix=gmix_make_exp6(pars, 6);
    } else if ( strcmp(object->model, "dev") ) {
        gmix=gmix_make_dev10(pars, 6);
    } else if ( strcmp(object->model, "gauss") ) {
        gmix=gmix_make_coellip(pars, 6);
    } else {
        fprintf(stderr,"bad object model: '%s'\n", object->model);
        exit(EXIT_FAILURE);
    }

    return gmix;
}
struct gmix *make_psf_gmix(struct object *object)
{
    double pars[6] = {};
    pars[0] = 1;
    pars[1] = 1;
    pars[2] = object->psf_e1;
    pars[3] = object->psf_e2;
    pars[4] = object->psf_T;
    pars[5] = 1;

    struct gmix *gmix=NULL;
    if ( strcmp(object->psf_model, "turb") ) {
        gmix=gmix_make_turb3(pars, 6);
    } else if ( strcmp(object->psf_model, "gauss") ) {
        gmix=gmix_make_coellip(pars, 6);
    } else {
        fprintf(stderr,"bad psf model: '%s'\n", object->psf_model);
        exit(EXIT_FAILURE);
    }

    return gmix;

}

struct gmix *make_gmix(struct object *object)
{
    struct gmix *gmix0    = make_object_gmix(object);
    struct gmix *gmix_psf = make_psf_gmix(object);
    struct gmix *gmix     = gmix_convolve(gmix0, gmix_psf);

    gmix0 = gmix_free(gmix0);
    gmix_psf = gmix_free(gmix_psf);

    return gmix;
}

void set_mask(struct image_mask *self, const struct gmix *gmix)
{
    double row=0, col=0;

    double T = gmix_get_T(gmix);
    double sigma = sqrt(T/2);

    double rad = GMIX_PADDING*sigma;

    gmix_get_cen(gmix, &row, &col);
    self->rowmin = row-rad;
    self->rowmax = row+rad;
    self->colmin = col-rad;
    self->colmax = col+rad;
}

void put_gmix(const struct gconfig *conf, struct image *image, const struct gmix *gmix)
{
    struct image_mask mask={0};
    set_mask(&mask, gmix);

    gmix_image_put_masked(image, gmix, conf->nsub, &mask);
}

void put_object(const struct gconfig *conf, struct image *image, struct object *object)
{
    struct gmix *gmix=make_gmix(object);

    put_gmix(conf, image, gmix);

    gmix=gmix_free(gmix);
}

void add_noise(const struct gconfig *conf, struct image *image)
{
    fprintf(stderr,"adding noise\n");
    if (strcmp(conf->noise_type,"poisson")) {
        fprintf(stderr,"Implement poisson noise\n");
        exit(EXIT_FAILURE);
    } else if (strcmp(conf->noise_type,"gauss"))  {
        double skysig=sqrt(conf->sky);
        image_add_randn(image, skysig);
    }
}

int main(int argc, char **argv)
{
    if (argc < 4) {
        fprintf(stderr,"gsim config_file catalog image\n");
        exit(EXIT_FAILURE);
    }
    const char *config_file=argv[1];
    const char *cat_file=argv[2];
    
    struct gconfig *conf=gconfig_read(config_file);
    gconfig_write(conf, stderr);
    FILE *cat_fptr=open_catalog(cat_file);

    struct image *image=make_image(conf);

    struct object object = {{0}};
    fprintf(stderr,"putting objects\n");
    while (object_read_one(&object, cat_fptr)) {
        put_object(conf, image, &object);
    }

    add_noise(conf, image);

    free(conf);
    image=image_free(image);
    fclose(cat_fptr);

}
