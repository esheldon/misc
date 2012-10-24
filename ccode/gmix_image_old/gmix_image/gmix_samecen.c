#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "gmix_samecen.h"
#include "image.h"
#include "gvec.h"
#include "defs.h"
#include "matrix.h"

/* 
 * in this version we force the centers to coincide.  This requires two
 * separate passes over the pixels, one for getting the new centers (and
 * setting the centroid to the mean) and then another calculating the
 * covariance matrix using the mean centroid
 *
 * This usually takes lots more iterations to converge compared to letting the
 * means float, but the answers are quite close...
 *
 */

int gmix_image_samecen(struct gmix* self,
                       struct image *image, 
                       struct gvec *gvec,
                       size_t *iter)
{
    int flags=0;
    size_t i=0;
    size_t ngauss = gvec->size;
    size_t nbytes = gvec->size*sizeof(double);
    double chi2=0, b=0;
    double u=0, v=0, uv=0, u2=0, v2=0, igrat=0;
    double gtot=0, imnorm=0, skysum=0.0, tau=0;
    double wmomlast=0, wmom=0, wmomdiff=0, psum=0;
    struct vec2 cen_new;

    struct gvec *gcopy=NULL;
    struct gauss* gauss=NULL;

    double sky=image_sky(image);
    double counts=image_counts(image);
    size_t npoints = IMSIZE(image);

    double nsky = sky/counts;
    double psky = sky/(counts/npoints);

    // these are all stack allocated

    double *gi = alloca(nbytes);
    double *trowsum = alloca(nbytes);
    double *tcolsum = alloca(nbytes);
    double *tu2sum = alloca(nbytes);
    double *tuvsum = alloca(nbytes);
    double *tv2sum = alloca(nbytes);

    // these need to be zeroed on each iteration
    double *pnew = alloca(nbytes);
    double *rowsum = alloca(nbytes);
    double *colsum = alloca(nbytes);
    double *u2sum = alloca(nbytes);
    double *uvsum = alloca(nbytes);
    double *v2sum = alloca(nbytes);

    gcopy = gvec_new(gvec->size);

    wmomlast=-9999;
    *iter=0;
    while (*iter < self->maxiter) {
        if (self->verbose)
            gvec_print(gvec,stderr);

        memset(rowsum,0,nbytes);
        memset(colsum,0,nbytes);
        memset(u2sum,0,nbytes);
        memset(uvsum,0,nbytes);
        memset(v2sum,0,nbytes);
        memset(pnew,0,nbytes);
        psum=0;
        skysum=0;

        // first to get centroid info
        for (int pass=1; pass<=2; pass++) {
            for (size_t col=0; col<image->ncols; col++) {
                for (size_t row=0; row<image->nrows; row++) {

                    imnorm=IMGET(image,row,col);
                    imnorm /= counts;

                    gtot=0;
                    gauss = &gvec->data[0];
                    for (i=0; i<ngauss; i++) {
                        if (gauss->det <= 0) {
                            wlog("found det: %.16g\n", gauss->det);
                            flags+=GMIX_ERROR_NEGATIVE_DET;
                            goto _gmix_image_samecen_bail;
                        }

                        // gi always evaluated using the center
                        // at beginning of this main iteration
                        u = (row-gauss->row);
                        v = (col-gauss->col);

                        u2 = u*u; v2 = v*v; uv = u*v;

                        chi2=gauss->icc*u2 + gauss->irr*v2 - 2.0*gauss->irc*uv;
                        chi2 /= gauss->det;
                        b = M_TWO_PI*sqrt(gauss->det);

                        gi[i] = gauss->p*exp( -0.5*chi2 )/b;
                        gtot += gi[i];

                        if (1==pass) {
                            trowsum[i] = row*gi[i];
                            tcolsum[i] = col*gi[i];
                        } else {
                            // after getting mean center, we do covar using
                            // this center but all else the same
                            u = (row-cen_new.v1);
                            v = (col-cen_new.v2);
                            u2 = u*u; v2 = v*v; uv = u*v;
                            tu2sum[i]  = u2*gi[i];
                            tuvsum[i]  = uv*gi[i];
                            tv2sum[i]  = v2*gi[i];
                        }

                        gauss++;
                    }
                    gtot += nsky;
                    igrat = imnorm/gtot;
                    for (i=0; i<ngauss; i++) {
                        // tau are same both passes
                        tau = gi[i]*igrat;  // Dave's tau*imnorm

                        if (1==pass) {
                            pnew[i] += tau;
                            psum += tau;
                            rowsum[i] += trowsum[i]*igrat;
                            colsum[i] += tcolsum[i]*igrat;
                        } else {
                            u2sum[i]  += tu2sum[i]*igrat;
                            uvsum[i]  += tuvsum[i]*igrat;
                            v2sum[i]  += tv2sum[i]*igrat;
                        }
                    }
                    if (1==pass) {
                        skysum += nsky*imnorm/gtot;
                    }

                } // rows
            } // cols

            if (1 == pass) {
                // the copy is only used for calculating the
                // mean center.  Will have new p and new centers
                // but old variances
                gvec_copy(gvec, gcopy);
                gmix_set_p_and_cen(gcopy, pnew, rowsum, colsum);
                // now calculate mean of new centers
                if (!gvec_wmean_center(gcopy, &cen_new)) {
                    flags += GMIX_ERROR_NEGATIVE_DET_SAMECEN;
                    goto _gmix_image_samecen_bail;
                }
                if (self->verbose)
                    wlog("newcen: %.16g %.16g\n", cen_new.v1, cen_new.v2);
            } else {
                gmix_set_mean_cen(gvec, &cen_new);
                gmix_set_p_and_mom(gvec,pnew,u2sum,uvsum,v2sum);
            }
        }
        psky = skysum;
        nsky = psky/npoints;

        wmom=0;
        gauss=gvec->data;
        for (i=0; i<ngauss; i++) {
            wmom += gauss->p*gauss->irr + gauss->p*gauss->icc;
            gauss++;
        }

        wmom /= psum;
        wmomdiff = fabs(wmom-wmomlast);
        if (self->verbose)
            wlog("iter: %lu  wmom: %.16g diffrat: %.16g\n", 
                    *iter, wmom, wmomdiff/wmom);
        if (wmomdiff/wmom < self->tol) {
            break;
        }
        wmomlast = wmom;
        (*iter)++;
    }

_gmix_image_samecen_bail:
    if (self->maxiter == (*iter)) {
        flags += GMIX_ERROR_MAXIT;
    }
    gvec_free(gcopy);
    return flags;
}


void gmix_set_mean_cen(struct gvec* gvec, struct vec2 *cen_mean)
{
    struct gauss *gauss=gvec->data;
    for (size_t i=0; i<gvec->size; i++) {
        gauss->row = cen_mean->v1;
        gauss->col = cen_mean->v2;
        gauss++;
    }

}

void gmix_set_p_and_cen(struct gvec* gvec, 
                        double* pnew,
                        double* rowsum,
                        double* colsum)
{
    struct gauss *gauss=gvec->data;
    for (size_t i=0; i<gvec->size; i++) {
        gauss->p = pnew[i];
        gauss->row = rowsum[i]/pnew[i];
        gauss->col = colsum[i]/pnew[i];
        gauss++;
    }

}

void gmix_set_p_and_mom(struct gvec* gvec, 
                        double* pnew,
                        double* u2sum,
                        double* uvsum,
                        double* v2sum)
{
    struct gauss *gauss=gvec->data;
    for (size_t i=0; i<gvec->size; i++) {
        gauss->p   = pnew[i];
        gauss->irr = u2sum[i]/pnew[i];
        gauss->irc = uvsum[i]/pnew[i];
        gauss->icc = v2sum[i]/pnew[i];
        gauss->det = gauss->irr*gauss->icc - gauss->irc*gauss->irc;
        gauss++;
    }

}


