#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "admom.h"
#include "admom_noise.h"
#include "gauss.h"
#include "image.h"
#include "randn.h"


void fill_gauss_image(struct image *image, const struct gauss *gauss)
{
    size_t nrows=0, ncols=0, row=0, col=0;
    double rowm=0,rowm2=0,colm=0,colm2=0,chi2=0;
    double *rowdata=NULL;

    nrows=IM_NROWS(image);
    ncols=IM_NCOLS(image);

    for (row=0; row<nrows; row++) {
        rowdata=IM_ROW(image, row);
        rowm = row-gauss->row;
        rowm2=rowm*rowm;

        for (col=0; col<ncols; col++) {

            colm = col-gauss->col;
            colm2 = colm*colm;

            chi2=gauss->dcc*rowm2 + gauss->drr*colm2 - 2.0*gauss->drc*rowm*colm;
            (*rowdata) = gauss->norm*gauss->p*exp( -0.5*chi2 );

            rowdata++;
        } // cols
    } // rows
}

int main(int argc, char **argv)
{

    struct am am = {{0}};
    struct image *im=NULL, *noisy_im=NULL;
    struct gauss gauss={0};
    //int row=10, col=10, irr=2.0, irc=0.1, icc=2.5;
    int row=10, col=10, irr=2., irc=0., icc=2.;
    time_t t1;
    double meandiff=0, imvar=0;
    double skysig=0, s2n_meas=0;
    double s2n=100;

    // make the true gaussian 
    if (!gauss_set(&gauss, 1.0, row, col, irr, irc, icc)) {
        fprintf(stderr,"bad gaussian determinant\n");
        exit(EXIT_FAILURE);
    }

    im=image_new(20,20);
    fill_gauss_image(im, &gauss);

    // we keep two images so we can check the variance is right
    // after adding noise
    noisy_im=image_copy(im);

    // use true gauss as weight to get s/n 
    (void) time(&t1);
    srand48((long) t1);
    admom_add_noise(noisy_im, s2n, &gauss, &skysig, &s2n_meas);
    image_compare(im, noisy_im, &meandiff, &imvar);

    fprintf(stderr,"s2n:      %.16g\n", s2n);
    fprintf(stderr,"s2n_meas: %.16g\n", s2n_meas);
    fprintf(stderr,"skysig:   %.16g\n", skysig);
    fprintf(stderr,"meandiff: %.16g\n", meandiff);
    fprintf(stderr,"std:      %.16g\n", sqrt(imvar));

    am.guess = gauss;
    am.nsigma = 4;
    am.maxiter = 100;
    am.shiftmax = 5;
    am.sky=0.0;
    am.skysig = skysig;
    fprintf(stderr,"am before\n");
    admom_print(&am, stderr);

    fprintf(stderr,"\n");
    admom(&am, noisy_im);
    admom_print(&am, stderr);

    im = image_free(im);
    noisy_im = image_free(noisy_im);
    return 0;
}
