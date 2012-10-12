#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "admom.h"
#include "amgauss.h"
#include "image.h"


double randn() 
{
    double x1, x2, w, y1;//, y2;
 
    do {
        x1 = 2.*drand48() - 1.0;
        x2 = 2.*drand48() - 1.0;
        w = x1*x1 + x2*x2;
    } while ( w >= 1.0 );

    w = sqrt( (-2.*log( w ) ) / w );
    y1 = x1*w;
    //y2 = x2*w;
    return y1;
}

double add_noise(struct image *image, double s2n, const struct amgauss *wt,
                 double *skysig, double *s2n_meas)
{
    size_t nrows=0, ncols=0, row=0, col=0, pass=0;
    double u=0,u2=0,v=0,v2=0,uv=0,chi2=0;
    double weight=0;
    double *rowdata=NULL;
    double sum=0, wsum=0, skysig=0, s2n_meas=0;

    nrows=IM_NROWS(image);
    ncols=IM_NCOLS(image);

    // first pass with noise=1
    *skysig=1.;
    *s2n_meas=-9999;
    for (pass=1,pass<=2;pass++) {

        sum=0;
        wsum=0;
        for (row=0; row<nrows; row++) {
            rowdata=IM_ROW(image, row);
            u = row-wt->row;
            u2=u*u;

            for (col=0; col<ncols; col++) {

                v = col-wt->col;
                v2 = v*v;
                uv = u*v;

                chi2=wt->dcc*u2 + wt->drr*v2 - 2.0*wt->drc*uv;
                weight = exp( -0.5*chi2 );

                sum += (*rowdata)*weight;
                wsum += weight;

                if (pass==2) {
                    (*rowdata) += (*skysig) * randn();
                }
                rowdata++;
            } // cols
        } // rows

        (*s2n_meas) = sum/sqrt(wsum)/skysig;
        if (pass==1) {
            // this new skysig should give us the requested S/N
            skysig = (*s2n_meas)/s2n * (*skysig);
        }
    }


}

void fill_gauss_image(struct image *image, const struct amgauss *gauss)
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
    struct image *im=NULL;
    struct amgauss gauss={0};
    int row=10, col=10, irr=2.0, irc=0.1, icc=2.5;

    im=image_new(20,20);

    // make the true gaussian 
    if (!amgauss_set(&gauss, 1.0, row, col, irr, irc, icc)) {
        fprintf(stderr,"bad gaussian determinant\n");
        exit(EXIT_FAILURE);
    }

    fill_gauss_image(im, &gauss);
}
