#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "image.h"
#include "gmix.h"
#include "gmix_image.h"
#include "gmix_em.h"

struct gmix *get_gmix(size_t ngauss)
{
    if (ngauss != 2) {
        fprintf(stderr,"only ngauss==2 for now\n");
        exit(EXIT_FAILURE);
    }
    struct gmix *gmix = gmix_new(ngauss);
    struct gauss *gptr = gmix->data;

    gauss_set(&gptr[0],
            0.6, 15., 15., 2.0, 0.0, 1.7);
    gauss_set(&gptr[1],
            0.4, 10., 8., 1.5, .3, 4.);
    return gmix;
}

struct gmix *get_guess_gmix(size_t ngauss, size_t nrow, size_t ncol)
{
    if (ngauss != 2) {
        fprintf(stderr,"only ngauss==2 for now\n");
        exit(EXIT_FAILURE);
    }
    struct gmix *gmix = gmix_new(ngauss);
    struct gauss *gptr = gmix->data;

    gauss_set(&gptr[0],
            0.27, 
            nrow/2. + 4*(drand48()-0.5),
            ncol/2. + 4*(drand48()-0.5),
            1.0, 
            0.0, 
            1.0);
    gauss_set(&gptr[1],
            0.23, 
            nrow/2. + 4*(drand48()-0.5),
            ncol/2. + 4*(drand48()-0.5),
            2.0, 
            0.0, 
            2.0);
    return gmix;
}


/*
void add_error_to_gmix(struct gmix *gmix)
{
    double fracerr=0.2;

    struct gauss *gauss=gmix->data;
    for (size_t i=0; i<gmix->size; i++) {
        double p =   gauss->p*  (1. + fracerr*(drand48()-0.5) );
        double row = gauss->row*(1. + fracerr*(drand48()-0.5) );
        double col = gauss->col*(1. + fracerr*(drand48()-0.5) );
        double irr = gauss->irr*(1. + fracerr*(drand48()-0.5) );
        double icc = gauss->icc*(1. + fracerr*(drand48()-0.5) );
        double irc = gauss->irc +     .1*(drand48()-0.5);

        gauss_set(gauss, p, row, col, irr, irc, icc);

        gauss++;
    }
}
*/
int main(int argc, char** argv)
{
    size_t ngauss=2;
    struct gmix_em gmix_em;
    size_t nrow=30, ncol=30;
    int nsub=1;

    time_t tm;
    (void) time(&tm);
    srand48((long) tm);

    /*
    if (argc != 2) {
        printf("usage: test image\n");
        exit(1);
    }
    */

    gmix_em.maxiter=2000;
    gmix_em.tol = 1.e-6;
    gmix_em.fixsky = 0;
    gmix_em.verbose=0;

    struct gmix *gmix_true = get_gmix(ngauss);
    wlog("True gmix:\n");
    gmix_print(gmix_true, stderr);
    wlog("\n\n");

    // our guess will also hold the anwer on output
    //struct gmix *gmix_guess = gmix_new_copy(gmix_true); 
    //add_error_to_gmix(gmix_guess);
    struct gmix *gmix_guess = get_guess_gmix(ngauss, nrow, ncol); 

    struct image* image = gmix_image_new(gmix_true, nrow, ncol, nsub);
    wlog("before sky image[15,15]: %.16g\n", IM_GET(image, 15, 15));
    wlog("before sky image[9,7]: %.16g\n", IM_GET(image, 9, 7));
    wlog("before sky counts: %.16g\n", IM_COUNTS(image));
    image_add_scalar(image, 1.0/IM_SIZE(image));

    wlog("nrows: %lu\n", IM_NROWS(image));
    wlog("ncols: %lu\n", IM_NCOLS(image));
    wlog("sky: %.16g\n", IM_SKY(image));
    wlog("image[15,15]: %.16g\n", IM_GET(image, 15, 15));
    wlog("image[9,7]: %.16g\n", IM_GET(image, 9, 7));
    wlog("counts: %.16g\n", IM_COUNTS(image));

    wlog("\ntrue\n");
    gmix_print(gmix_true,stderr);
    wlog("guess\n");
    gmix_print(gmix_guess,stderr);

    // new gmix to be filled, but starting with the guess
    // this is so we can keep our guess around for comparison
    struct gmix *gmix=gmix_new_copy(gmix_guess);
    gmix_em_run(&gmix_em, image, gmix);

    wlog("\nnumiter: %lu\n", gmix_em.numiter);
    if (gmix_em.flags != 0) {
        wlog("failure with flags: %d\n", gmix_em.flags);
    } else {

        wlog("measured\n");
        gmix_print(gmix,stderr);
    }
}
