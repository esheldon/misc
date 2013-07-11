#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "image.h"
#include "gmix.h"
#include "gmix_image.h"
#include "gmix_em.h"

struct gmix *get_gmix(size_t ngauss, size_t nrow, size_t ncol)
{
    if (ngauss != 2) {
        wlog("only ngauss==2 for now\n");
        exit(EXIT_FAILURE);
    }
    struct gmix *gmix = gmix_new(ngauss);
    struct gauss *gptr = gmix->data;

    double col=nrow/2.;
    double row=ncol/2.;

    gauss2_set(&gptr[0],
            0.6, row, col, 7., 0.0, 12.);
    gauss2_set(&gptr[1],
            0.4, row, col, 16., 5., 27.);
    return gmix;
}

struct gmix *get_guess_gmix(size_t ngauss, size_t nrow, size_t ncol)
{
    if (ngauss != 2) {
        wlog("only ngauss==2 for now\n");
        exit(EXIT_FAILURE);
    }
    struct gmix *gmix = gmix_new(ngauss);
    struct gauss *gptr = gmix->data;

    double row=nrow/2. + 2*(drand48()-0.5);
    double col=ncol/2. + 2*(drand48()-0.5);

    gauss2_set(&gptr[0],
            0.2 + .05*(drand48()-0.5), 
            row,
            col,
            10.0*(1 + .1*(drand48()-0.5)), 
            0.0 + 2*(drand48()-0.5), 
            10.0*(1 + .1*(drand48()-0.5)) );
    gauss2_set(&gptr[1],
            0.3 + .05*(drand48()-0.5), 
            row,
            col,
            18.0*(1. + .1*(drand48()-0.5)), 
            0.0 + 2*(drand48()-0.5), 
            18.0*(1. + .1*(drand48()-0.5))  );
    return gmix;
}

int main(int argc, char** argv)
{
    char fname[] = "test-image.dat";
    char fit_fname[] = "test-image-fit.dat";
    size_t ngauss=2;
    size_t nrow=40, ncol=40;
    int nsub=1;

    time_t tm;
    (void) time(&tm);
    srand48((long) tm);

    // the true gmix
    struct gmix *gmix_true = get_gmix(ngauss, nrow, ncol);

    // make the image
    struct image* image = gmix_image_new(gmix_true, nrow, ncol, nsub);
    wlog("storing image in '%s'\n", fname);
    {
        FILE *fobj=fopen(fname,"w");
        image_write(image, fobj);
        fclose(fobj);
    }

    // the "self"
    struct gmix_em gmix_em;
    gmix_em.maxiter=4000;
    gmix_em.tol = 1.e-6;
    gmix_em.verbose=0;


    // this is purposefully bad in general
    struct gmix *gmix = get_guess_gmix(ngauss, nrow, ncol); 

    double counts=image_get_counts(image);
    double sky2add=1.0/IM_SIZE(image);

    wlog("before sky image[15,15]: %.16g\n", IM_GET(image, 15, 15));
    wlog("before sky image[9,7]: %.16g\n", IM_GET(image, 9, 7));
    wlog("before sky counts: %.16g\n", counts);

    // now add some sky needed by the gmix alorithm
    image_add_scalar(image, sky2add);
    counts=image_get_counts(image);

    wlog("added sky: %.16g\n", sky2add);
    wlog("nrows: %lu\n", IM_NROWS(image));
    wlog("ncols: %lu\n", IM_NCOLS(image));
    wlog("sky: %.16g\n", IM_SKY(image));
    wlog("image[15,15]: %.16g\n", IM_GET(image, 15, 15));
    wlog("image[9,7]: %.16g\n", IM_GET(image, 9, 7));
    wlog("counts: %.16g\n", counts);

    // for comparison; gmix will change
    wlog("\ntrue\n");
    gmix_print(gmix_true,stderr);
    wlog("guess\n");
    gmix_print(gmix,stderr);

    gmix_em_cocenter_run(&gmix_em, image, gmix);

    wlog("\nnumiter: %lu\n", gmix_em.numiter);
    if (gmix_em.flags != 0) {
        wlog("failure with flags: %d\n", gmix_em.flags);
    } else {
        wlog("measured\n");
        gmix_print(gmix,stderr);

        wlog("storing fit image in '%s'\n", fit_fname);
        {
            FILE *fobj=fopen(fit_fname,"w");
            struct image* model_image = gmix_image_new(gmix, nrow, ncol, nsub);
            image_write(model_image, fobj);
            fclose(fobj);
        }
    }

}
