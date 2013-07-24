#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "admom.h"
#include "gauss2.h"
#include "image.h"
#include "randn.h"


void fill_gauss_image(struct image *image, const struct gauss2 *gauss)
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
    struct gauss2 gauss={0};
    time_t t1;
    double meandiff=0, imvar=0;
    double skysig=0, s2n_meas=0;
    double s2n=40;
    size_t itrial=0,ntrial=1; // different random realizations

    if (argc > 1) {
        ntrial = atoi(argv[1]);
    }

    int nrows=21;
    int ncols=21;
    double row=(nrows-1.)/2.;
    double col=(ncols-1.)/2.;
    double irr=2.;
    double irc=0.;
    double icc=2.;

    // make the true gaussian 
    if (!gauss2_set(&gauss, 1.0, row, col, irr, irc, icc)) {
        fprintf(stderr,"bad gaussian determinant\n");
        exit(EXIT_FAILURE);
    }

    im=image_new(nrows,ncols);
    noisy_im=image_new(nrows,ncols);

    fill_gauss_image(im, &gauss);

    // use true gauss as weight to get s/n 
    (void) time(&t1);
    srand48((long) t1);

    fprintf(stderr,"s2n:      %.16g\n", s2n);
    fprintf(stderr,"s2n_meas: %.16g\n", s2n_meas);
    //fprintf(stderr,"skysig:   %.16g\n", skysig);
    fprintf(stderr,"meandiff: %.16g\n", meandiff);
    fprintf(stderr,"std:      %.16g\n", sqrt(imvar));

    am.nsigma = 4;
    am.maxiter = 100;
    am.shiftmax = 5;
    am.sky=0.0;

    for (itrial=0; itrial<ntrial; itrial++) {

        image_copy(im, noisy_im);
        admom_add_noise(noisy_im, s2n, &gauss, &skysig, &s2n_meas);
        am.skysig = skysig;

        gauss2_set(&am.guess,
                  1.,
                  gauss.row + 2*(drand48()-0.5),
                  gauss.col + 2*(drand48()-0.5),
                  gauss.irr + gauss.irr*(drand48()-0.5),
                  0.,
                  gauss.icc + gauss.icc*(drand48()-0.5));

        fprintf(stderr,"am before\n");
        admom_print(&am, stderr);
        fprintf(stderr,"\n");

        admom(&am, noisy_im);
        if (am.flags != 0) {
            fprintf(stderr,"error encountered\n");
        } else {

            fprintf(stderr,"\nam after\n");
            admom_print(&am, stderr);
            // this goes to stdout so we can use it
        }
        printf("%d %.16g %.16g %.16g %.16g\n", am.flags, am.wt.e1, am.wt.e2, gauss.e1, gauss.e2);
    }

    im = image_free(im);
    noisy_im = image_free(noisy_im);
    return 0;
}
