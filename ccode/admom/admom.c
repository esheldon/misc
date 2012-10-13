/* 
   Adaptive moments.  no sub-pixel integration in this version


*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "admom.h"
#include "gauss.h"
#include "image.h"

/* 
   calculate the mask covering a certain number
   of sigma around the center.  Be careful of
   the edges
*/
static void get_mask(const struct image *image,
                     struct gauss *gauss, 
                     double nsigma,
                     struct bound *mask)
{
    double rowmin=0,rowmax=0,colmin=0,colmax=0;
    double grad=0;

    grad = nsigma*sqrt(fmax(gauss->irr,gauss->icc));
    rowmin = lround(fmax( gauss->row-grad-0.5,0.) );
    rowmax = lround(fmin( gauss->row+grad+0.5, (double)IM_NROWS(image)-1 ) );

    colmin = lround(fmax( gauss->col-grad-0.5,0.) );
    colmax = lround(fmin( gauss->col+grad+0.5, (double)IM_NCOLS(image)-1 ) );

    bound_set(mask, rowmin, rowmax, colmin, colmax);

}

/* 
   Two passes: update the weighted center then measure the
   adaptive moments.
*/
static void calc_moments(struct am *am, const struct image *image)
{
    size_t row=0,col=0, nrows=0, ncols=0;
    double sum=0,rowsum=0,colsum=0,wsum=0,row2sum=0,col2sum=0,rowcolsum=0;
    double rowm=0,rowm2=0,colm=0,colm2=0,ymod=0,weight=0,expon=0;
    double sums4=0,det25=0;
    double wtrow=0, wtcol=0;
    const double *rowdata=NULL;
    struct gauss *wt=NULL;
    const struct gauss *guess=NULL;
    int pass=0;

    nrows=IM_NROWS(image);
    ncols=IM_NCOLS(image);

    wt=&am->wt;

    for (pass=1; pass<=2; pass++ ) {
        wtrow=wt->row - IM_ROW0(image);
        wtcol=wt->col - IM_COL0(image);

        sum=0; wsum=0; rowsum=0; colsum=0; row2sum=0; 
        col2sum=0; rowcolsum=0; sums4=0;
        for (row=0; row<nrows; row++) {
            // use IM_ROW because the image can be masked
            rowdata=IM_ROW(image,row);
            rowm = row - wtrow;
            rowm2=rowm*rowm;

            for (col=0; col<ncols; col++) {
                colm = col - wtcol;
                colm2 = colm*colm;

                expon=wt->dcc*rowm2 + wt->drr*colm2 - 2.*wt->drc*rowm*colm;
                weight=exp(-0.5*expon);

                // must use IM_GET because image could be masked
                ymod = (*rowdata);
                ymod -= am->sky;
                ymod *= weight;

                sum += ymod;
                wsum += weight;

                if (pass==1) {
                    rowsum += row*ymod;
                    colsum += col*ymod;
                } else {
                    row2sum   += rowm2*ymod;
                    col2sum   += colm2*ymod;
                    rowcolsum += rowm*colm*ymod;
                    sums4     += expon*expon*ymod;
                }

                rowdata++;
            }
        }
        if (sum <= 0 || wsum <= 0) { 
            DBG fprintf(stderr,"error: sum <= 0 || wsum <= 0\n");
            am->flags |= AM_FLAG_FAINT;
            break;
        }

        if (pass==1) {
            wt->row=IM_ROW0(image) + rowsum/sum;
            wt->col=IM_COL0(image) + colsum/sum;
            guess=&am->guess;
            if ((fabs(wt->row-guess->row) > am->shiftmax)
                    || (fabs(wt->col-guess->col) > am->shiftmax) ) {
                DBG fprintf(stderr,"error: centroid shift\n");
                DBG fprintf(stderr,"  row: %.16g -> %.16g\n",
                        guess->row,wt->row);
                DBG fprintf(stderr,"  col: %.16g -> %.16g\n",
                        guess->col,wt->col);
                am->flags |= AM_FLAG_SHIFT;
                break;
            }
        } else {
            am->s2n=sum/sqrt(wsum)/am->skysig;
            am->irr_tmp = row2sum/sum;
            am->irc_tmp = rowcolsum/sum;
            am->icc_tmp = col2sum/sum;
            am->det_tmp = am->irr_tmp*am->icc_tmp - am->irc_tmp*am->irc_tmp;
            if (am->irr_tmp <= 0 || am->icc_tmp <= 0 || am->det_tmp <= AM_DETTOL) {
                DBG fprintf(stderr,"error: measured mom <= 0 or det <= %.16g\n",AM_DETTOL);
                DBG fprintf(stderr,"  irr: %.16g\n  icc: %.16g\n  det: %.16g\n",
                            am->irr_tmp, am->icc_tmp, am->det_tmp);
                am->flags |= AM_FLAG_FAINT;
                break;
            }

            am->rho4 = sums4/sum;

            det25 = pow(wt->det, 0.25);
            am->uncer=4.*sqrt(M_PI)*am->skysig*det25/(4.*sum - sums4);
        }

    } // passes

}


// take the adaptive step.  Update the weight parameters.
static void admom_step(struct am *am) {
    struct gauss *wt=&am->wt;

    double detm_inv = 1./am->det_tmp;
    double detw_inv = 1./wt->det;

    double nicc = am->irr_tmp*detm_inv - wt->irr*detw_inv;
            //n(1,1)=m(2,2)*detm-w(2,2)*detw
    double nirr = am->icc_tmp*detm_inv - wt->icc*detw_inv;
            //n(2,2)=m(1,1)*detm-w(1,1)*detw
    double nirc = -am->irc_tmp*detm_inv + wt->irc*detw_inv;
            //n(1,2)=-m(1,2)*detm+w(1,2)*detw
    double detn=nirr*nicc - nirc*nirc;

    if (detn <= 0.) {
        DBG fprintf(stderr,"error: detn %.16g <= 0\n", detn);
        am->flags |= AM_FLAG_FAINT;
        return;
    }

    double detn_inv = 1./detn;
    double icc =  nirr*detn_inv;
    double irc = -nirc*detn_inv;
    double irr =  nicc*detn_inv;
    gauss_set(wt,
              wt->p, wt->row, wt->col,
              irr, irc, icc);
}

void admom_print(const struct am *am, FILE *stream)
{
    fprintf(stderr,"  - guess gauss:\n");
    gauss_print(&am->guess, stream);
    fprintf(stderr,"  - weight gauss:\n");
    gauss_print(&am->wt, stream);

    fprintf(stderr,"  rho4:    %.16g\n", am->rho4);
    fprintf(stderr,"  s2n:     %.16g\n", am->s2n);
    fprintf(stderr,"  uncer:   %.16g\n", am->uncer);
    fprintf(stderr,"  numiter: %d\n", am->numiter);
    fprintf(stderr,"  flags:   %d\n", am->flags);
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

    struct gauss *wt=NULL;
    const struct gauss *guess=NULL;

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
        am->numiter++;

        get_mask(image, wt, am->nsigma, &mask);
        image_add_mask(maskim, &mask, 0); // 0 means don't update counts

        //fprintf(stderr,"mask:\n");
        //bound_print(&mask, stderr);

        //update_cen(am, maskim);
        calc_moments(am, maskim);
        if (am->flags != 0) {
            goto _admom_bail;
        }
        if (fabs(wt->e1-e1old) < AM_TOL1
                && fabs(wt->e2-e2old) < AM_TOL1
                && fabs(wt->irr/irrold-1.) < AM_TOL2) {
            break;
        }

        e1old = am->wt.e1;
        e2old = am->wt.e2;
        irrold = am->wt.irr;
        admom_step(am);
        if (am->flags != 0) {
            goto _admom_bail;
        }
    }

    if (am->numiter >= am->maxiter) {
        DBG fprintf(stderr,"error: maxit reached\n");
        am->flags |= AM_FLAG_MAXIT;
        goto _admom_bail;
    }


_admom_bail:
    // does not clear memory for original referenced image
    maskim=image_free(maskim);
}





