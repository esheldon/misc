/* 
   Adaptive moments.  no sub-pixel integration in this version


*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "admom.h"
#include "amgauss.h"
#include "image.h"

/* 
   calculate the mask covering a certain number
   of sigma around the center.  Be careful of
   the edges
*/
static void get_mask(const struct image *image,
                     struct amgauss *gauss, 
                     struct bound *mask)
{
    double rowmin=0,rowmax=0,colmin=0,colmax=0;
    double grad=0;

    grad = 4.*sqrt(fmax(gauss->irr,gauss->icc));
    rowmin = lround(fmax( gauss->row-grad-0.5,1.) );
    rowmax = lround(fmin( gauss->row+grad+0.5, (double)IM_NROWS(image)-1 ) );

    colmin = lround(fmax( gauss->col-grad-0.5,1.) );
    colmax = lround(fmin( gauss->col+grad+0.5, (double)IM_NCOLS(image)-1 ) );

    bound_set(mask, rowmin, rowmax, colmin, colmax);

}

/* update the weighted center.  Set flags if s2n < 0 or
   the centroid has shifted too far */
static void update_cen(struct am *am, const struct image *image)
{
    int row=0,col=0;
    double sum=0, rowsum=0, colsum=0, wsum=0;
    double rowm=0, colm=0, ymod=0;
    const double *rowdata=NULL;
    double weight=0, expon=0;
    struct amgauss *wt=NULL;
    const struct amgauss *guess=NULL;

    size_t nrows=0, ncols=0;

    nrows=IM_NROWS(image);
    ncols=IM_NCOLS(image);

    wt=&am->wt;

    for (row=0; row<=nrows; row++) {
        // use IM_ROW because the image can be masked
        rowdata=IM_ROW(image,row);
        rowm = row - wt->row;

        for (col=0; col<=ncols; col++) {
            colm = col - wt->col;

            expon=wt->dcc*rowm*rowm + wt->drr*colm*colm - 2.*wt->drc*rowm*colm;
            weight=exp(-0.5*expon);

            // must use IM_GET because image could be masked
            ymod = (*rowdata);
            ymod -= am->sky;
            ymod *= weight;

            sum += ymod;
            rowsum += row*ymod;
            colsum += col*ymod;
            wsum += weight;

            rowdata++;
        }
    }
    wt->row=rowsum/wsum;
    wt->col=colsum/wsum;
    am->s2n=sum/sqrt(wsum)/am->skysig;

    if (am->s2n < 0) { 
        am->flags |= AM_FLAG_FAINT;
    }

    guess=&am->guess;
    if ((fabs(wt->row-guess->row) > am->shiftmax)
            || (fabs(wt->col-guess->col) > am->shiftmax) ) {
        am->flags |= AM_FLAG_SHIFT;
    }

}

static void calc_moments(struct am *am, const struct image *image) {

}

static void admom_step(struct am *am) {

}


/*
   The guess gaussian should be set
     row,col,irr,irc,icc should be the starting guess
   Also in the am struct these should be set
     maxiter,shiftmax,sky,skysig

   On output the parameters of the gaussian are updated and flags are set.
*/

void admom(struct am *am, const struct image *image)
{

    struct amgauss *wt=NULL;
    const struct amgauss *guess=NULL;

    //double mrr=0, mrc=0, mcc=0;
    //double nrr=0, nrc=0, ncc=0;
    double e1old=0, e2old=0, irrold=0;

    int iter=0;
    struct image *maskim = NULL;
    struct bound mask = {0};

    // For masking.  won't own data
    maskim = image_getref(image);

    wt=&am->wt;
    guess=&am->guess;

    // start with weight gauss equal to guess
    *wt = *guess;
    wt->irr = fmax(1.0, wt->irr);
    wt->icc = fmax(1.0, wt->icc);

    e1old=10.;
    e2old=10.;
    irrold=1.e6;
 
    am->numiter=0;
    am->flags=0;
    for (iter=0; iter<am->maxiter; iter++) {

        get_mask(image, wt, &mask);
        image_add_mask(maskim, &mask);

        update_cen(am, maskim);
        if (am->flags != 0) {
            goto _admom_bail;
        }

        calc_moments(am, maskim);
        if (am->flags != 0) {
            goto _admom_bail;
        }
        if (fabs(wt->e1-e1old) < AM_TOL1
                && fabs(wt->e2-e2old) < AM_TOL1
                && fabs(wt->irr/irrold-1.) < AM_TOL2) {
            break;
        }

        admom_step(am);
        am->numiter++;
    }

    if (am->numiter >= am->maxiter) {
        am->flags |= AM_FLAG_MAXIT;
        goto _admom_bail;
    }


_admom_bail:
    // does not clear memory for original referenced image
    maskim=image_free(maskim);
}





