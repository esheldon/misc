#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "image.h"
#include "randn.h"
#include "gauss.h"

void admom_add_noise(struct image *image, double s2n, const struct gauss *wt,
                     double *skysig, double *s2n_meas)
{
    size_t nrows=0, ncols=0, row=0, col=0, pass=0;
    double u=0,u2=0,v=0,v2=0,uv=0,chi2=0;
    double weight=0;
    double *rowdata=NULL;
    double sum=0, wsum=0;

    nrows=IM_NROWS(image);
    ncols=IM_NCOLS(image);

    // first pass with noise=1
    (*skysig)=1.;
    (*s2n_meas)=-9999;
    for (pass=1;pass<=2;pass++) {

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

        (*s2n_meas) = sum/sqrt(wsum)/(*skysig);
        if (pass==1) {
            // this new skysig should give us the requested S/N
            (*skysig) = (*s2n_meas)/s2n * (*skysig);
        }
    }
}


