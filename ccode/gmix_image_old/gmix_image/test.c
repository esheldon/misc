#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "image.h"
#include "gvec.h"
#include "gmix_image.h"
#include "defs.h"

int main(int argc, char** argv)
{
    size_t ngauss=2;
    struct gvec *ginit, *gvec;
    struct image *image;
    struct gmix gmix;
    struct gauss *gptr=NULL;
    size_t niter;
    struct timespec ts_start;
    struct timespec ts_end;

    gmix.maxiter=1000;
    gmix.tol = 1.e-6;
    //gmix.tol = 1.e-5;
    gmix.fixsky = 1;
    gmix.verbose=0;

    ginit = gvec_new(ngauss);
    gptr = ginit->data;

    gptr[0].p = 0.6;
    gptr[0].irr=2.0;
    gptr[0].irc=0.0;
    gptr[0].icc=2.0;
    gptr[0].row = 15;
    gptr[0].col = 15;

    gptr[1].p = 0.4;
    gptr[1].irr=1.5;
    gptr[1].irc=0.3;
    gptr[1].icc=4.0;
    gptr[1].row = 10;
    gptr[1].col = 8;
 
    // set the deteminants
    gvec_set_dets(ginit);
    //image = image_new(31,31);
    image = image_read_text("/astro/u/esheldon/tmp/timage-sky-noisy.dat");
    //image = image_read_text("/astro/u/esheldon/tmp/timage-sky.dat");
    if (image==NULL)
        exit(EXIT_FAILURE);
    wlog("image[7,9]: %.16g\n", IMGET(image, 7, 9));
    wlog("image[9,7]: %.16g\n", IMGET(image, 9, 7));

    gvec = gvec_new(ginit->size);
    gvec_copy(ginit, gvec);

    clock_gettime(CLOCK_MONOTONIC, &ts_start);
    int flags = gmix_image(&gmix, image, gvec, &niter);
    //int flags = gmix_image_old(&gmix, image, gvec, &niter);
    clock_gettime(CLOCK_MONOTONIC, &ts_end);
    double sec = 
        ((double)ts_end.tv_sec-ts_start.tv_sec)
        +
        1.e-9*((double)(ts_end.tv_nsec-ts_start.tv_nsec));
    wlog("time: %.7lf sec\n", sec);


    wlog("numiter: %lu\n", niter);
    if (flags != 0) {
        wlog("failure with flags: %d\n", flags);
    } else {

        gptr = ginit->data;
        wlog("input\n");
        gvec_print(ginit,stderr);

        wlog("meas\n");
        gvec_print(gvec,stderr);
    }
}
