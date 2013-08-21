/*

    gsim config_file catalog image

    Generate a simulated image with parameters specified in the config file.
    
    read object info from the catalog file (one per line), place them in an
    image with noise and write to the image file.

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
            "gauss" for gaussian noise or "poisson"

            For poisson noise, a deviate is drawn based on
            the value sky+flux in each pixel.

            For gaussian, sqrt(sky) is used for the width of gaussian noise.

        sky = double
            Value for the sky.  Specify zero for no noise.
        nsub = integer
            the number of points in each dimension to use for sub-pixel
            integration over the pixels.

        ellip_type = "e" or "g"
            if "e" then the ellipticities in the file are given as
            (a^2-b^2)/(a^2+b^2).  If ellip_type is "g" they are (a-b)/(a+b)

        seed = integer
            Seed for the random number generator


    catalog file
    ------------

    The catalog file should be a space-delimited text file with the following
    columns

        model row col e1 e2 sigma counts psfmodel psfe1 psfe2 psf_sigma

    Note that for psfs, the centroid and counts are not given

    column description

        - model should currently be 
            "gauss" - a single gaussian
            "exp"   - an exponential represented using gaussians
            "dev"   - an devaucouleur represented using gaussians
            "turb"  - turbulent psf represented using gaussians
            "psf"   - the object ellip/size info is ignored and a psf is placed
                      at the given location

          If you want a bulge+disk, just put two entries in the file.  You can
          add any number of components to an object this way.

        - row,col
            zero-offset row and column in the image

        - e1,e2
            The ellipticity of the object or the psf.  This is the version defined 
            by the <x^2>

                T=<x^2> + <y^2>
                e1=(<x^2> - <y^2>)/T
                e2=2*<xy>/T

            We could just as easily use the reduced shear definition.

        - sigma is the "sigma" of the object or psf, defined as
            sqrt(  (<x^2> + <y^2>)/2 ) === sqrt(T/2)
          for a gaussian this is the average of the sigmas in each dimension
   
    output image
    ------------
    The output image is written to the indicated file.  The format
    is FITS.

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "gconfig.h"
#include "object_simple.h"
#include "catalog.h"
#include "image.h"
#include "gmix.h"
#include "gmix_image.h"
#include "image_rand.h"
#include "image_fits.h"

// we will only draw the objects out do a radius of GMIX_PADDING*sigma
//
// sigma is the average over the gaussians in the mixture
//
//     T = <x^2> + <y^2>
//    <T> = sum(p_i * T_i)/sum(p_i)
//    sigma = sqrt(T/2)
//
// draw_radius = GMIX_PADDING*sigma
#define GMIX_PADDING 5.0

/* 
   make sure this is the same random number generator used by image_rand.c
*/

void set_seed(long seed) {
    srand48(seed);
}

struct image *make_image(const struct gconfig *conf)
{
    fprintf(stderr,"making image\n");
    struct image *im=image_new(conf->nrow, conf->ncol);
    return im;
}


void add_noise(const struct gconfig *conf, struct image *image)
{
    if (conf->sky <= 0) {
        fprintf(stderr,"sky is zero, not adding sky or noise\n");
        return;
    }

    fprintf(stderr,"adding noise\n");
    fprintf(stderr,"    adding sky background\n");
    image_add_scalar(image,conf->sky);

    // strcasecmp is gnu extension
    if (0==strcasecmp(conf->noise_type,"none")) {
        fprintf(stderr,"    not adding noise\n");
        return;
    }

    if (0==strcasecmp(conf->noise_type,"poisson")) {
        fprintf(stderr,"    adding poisson noise\n");
        image_add_poisson(image);
    } else if (0==strcasecmp(conf->noise_type,"gauss")) {
        fprintf(stderr,"    adding gaussian noise\n");
        double skysig=sqrt(conf->sky);
        image_add_randn(image, skysig);
    } else {
        // we check the config, so should not get here
        fprintf(stderr,"unexpected noise_type: '%s'\n", conf->noise_type);
        exit(EXIT_FAILURE);
    }
}

/* add a ! to front of name so cfitsio will clobber any existing file */
void write_image(const struct image *self,
                 const char *filename)
{
    int clobber=1;
    fprintf(stderr,"writing %s\n", filename);
    image_write_fits(self, filename, clobber);
}

struct gmix *make_gmix0(struct object_simple *object)
{
    double pars[6] = {0};
    long flags=0;

    pars[0] = object->row;
    pars[1] = object->col;
    pars[2] = object->shape.e1;
    pars[3] = object->shape.e2;
    pars[4] = object->T;
    pars[5] = object->counts;

    struct gmix_pars *gpars=NULL;
    struct gmix *gmix=NULL;

    if ( 0==strcasecmp(object->model, "exp") ) {
        gpars=gmix_pars_new(GMIX_EXP, pars, 6, SHAPE_SYSTEM_E, &flags);
    } else if ( 0==strcasecmp(object->model, "dev") ) {
        gpars=gmix_pars_new(GMIX_DEV, pars, 6, SHAPE_SYSTEM_E, &flags);
    } else if ( 0==strcasecmp(object->model, "gauss") ) {
        gpars=gmix_pars_new(GMIX_COELLIP, pars, 6, SHAPE_SYSTEM_E, &flags);
    } else {
        fprintf(stderr,"bad object model: '%s'\n", object->model);
        exit(EXIT_FAILURE);
    }

    if (flags != 0) {
        fprintf(stderr,"problem making pars: %s: %d\n",
                __FILE__,__LINE__);
        exit(1);
    }


    gmix = gmix_new_model(gpars, &flags);
    if (flags != 0) {
        fprintf(stderr,"problem making model: %s: %d\n",
                __FILE__,__LINE__);
        exit(1);
    }


    gpars=gmix_pars_free(gpars);
    return gmix;
}


struct gmix *make_psf_gmix(struct object_simple *object, 
                           double row, double col, 
                           double flux)
{
    double pars[6] = {0};
    long flags=0;
    pars[0] = row;
    pars[1] = col;
    pars[2] = object->psf_shape.e1;
    pars[3] = object->psf_shape.e2;
    pars[4] = object->psf_T;
    pars[5] = flux;

    struct gmix_pars *gpars=NULL;
    struct gmix *gmix=NULL;

    if ( 0==strcasecmp(object->psf_model, "turb") ) {
        gpars=gmix_pars_new(GMIX_TURB, pars, 6, SHAPE_SYSTEM_E, &flags);
    } else if ( 0==strcasecmp(object->psf_model, "gauss") ) {
        gpars=gmix_pars_new(GMIX_COELLIP, pars, 6, SHAPE_SYSTEM_E, &flags);
    } else {
        fprintf(stderr,"bad psf model: '%s'\n", object->psf_model);
        exit(EXIT_FAILURE);
    }

    if (flags != 0) {
        fprintf(stderr,"problem making psf pars: %s: %d\n",
                __FILE__,__LINE__);
        exit(1);
    }

    gmix = gmix_new_model(gpars, &flags);
    if (flags != 0) {
        fprintf(stderr,"problem making psf model: %s: %d\n",
                __FILE__,__LINE__);
        exit(1);
    }


    gpars=gmix_pars_free(gpars);
    return gmix;

}

struct gmix *make_star_gmix(struct object_simple *object)
{
    struct gmix *gmix=make_psf_gmix(object,
                                    object->row, 
                                    object->col,
                                    object->counts);
    return gmix;
}


struct gmix *make_galaxy_gmix(struct object_simple *object)
{
    long flags=0;
    struct gmix *gmix0    = make_gmix0(object);
    struct gmix *gmix_psf = make_psf_gmix(object,1,1,1);
    struct gmix *gmix     = gmix_convolve(gmix0, gmix_psf,&flags);

    gmix0 = gmix_free(gmix0);
    gmix_psf = gmix_free(gmix_psf);

    return gmix;
}


struct gmix *make_gmix(struct object_simple *object)
{
    struct gmix *gmix = NULL;

    if (0==strcasecmp(object->model, "star")) {
        gmix = make_star_gmix(object);
    } else {
        if (0==strcasecmp(object->psf_model,"none")) {
            gmix = make_gmix0(object);
        } else {
            gmix = make_galaxy_gmix(object);
        }
    }

    return gmix;
}

void get_radius_and_cen(const struct gmix *gmix, 
                        double *rad, double *row, double *col)
{
    double irr=0,irc=0,icc=0,counts=0;

    gmix_get_totals(gmix, row, col, &irr, &irc, &icc, &counts);
    if (irr > icc) {
        (*rad) = sqrt(irr);
    } else {
        (*rad) = sqrt(icc);
    }

    (*rad) *= GMIX_PADDING;
}
void set_mask(struct image_mask *self, const struct gmix *gmix)
{
    double row=0, col=0, rad=0;
    get_radius_and_cen(gmix, &row, &col, &rad);

    get_radius_and_cen(gmix, &rad, &row, &col);
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

void put_object(const struct gconfig *conf, struct image *image, struct object_simple *object)
{
    struct gmix *gmix=make_gmix(object);

    put_gmix(conf, image, gmix);

    gmix=gmix_free(gmix);
}

void put_objects(const struct gconfig *conf, struct image *image, struct catalog *cat)
{
    fprintf(stderr,"putting objects\n");
    struct object_simple *object = cat->data;
    for (ssize_t i=0; i<cat->size; i++) {
        if ( ((i+1) % 500) ==0 || i==0 ) {
            fprintf(stderr,"%ld/%ld\n", i+1, cat->size);
        }
        put_object(conf, image, object);
        object++;
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
    const char *image_file=argv[3];

    
    struct gconfig *conf=gconfig_read(config_file);
    gconfig_write(conf, stderr);

    set_seed(conf->seed);

    struct catalog *cat=catalog_read(cat_file);
    struct image *image=make_image(conf);

    put_objects(conf, image, cat);
    add_noise(conf, image);

    write_image(image, image_file);
    gconfg_write2fits(conf, image_file);

    free(conf);
    image=image_free(image);
    cat=catalog_free(cat);
}
