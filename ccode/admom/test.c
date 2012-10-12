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
    double u=0,u2=0,v=0,v2=0,uv=0,chi2=0;
    double *rowdata=NULL;

    nrows=IM_NROWS(image);
    ncols=IM_NCOLS(image);

    for (row=0; row<nrows; row++) {
        rowdata=IM_ROW(image, row);
        u = row-gauss->row;
        u2=u*u;

        for (col=0; col<ncols; col++) {

            v = col-gauss->col;
            v2 = v*v;
            uv = u*v;

            chi2=gauss->dcc*u2 + gauss->drr*v2 - 2.0*gauss->drc*uv;
            (*rowdata) = gauss->norm*gauss->p*exp( -0.5*chi2 );

            rowdata++;
        } // cols
    } // rows
}

int main(int argc, char **argv)
{

    //struct am am = {0};
    struct image *im=NULL, *noisy_im=NULL;
    struct gauss gauss={0};
    int row=10, col=10, irr=2.0, irc=0.1, icc=2.5;
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

    return 0;
}
