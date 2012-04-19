/*
 
    Algorithm is simple:

    Start with a guess for the N gaussians.

    Then the new estimate for the gaussian weight "p" for gaussian gi is

        pnew[gi] = sum_pix( gi[pix]/gtot[pix]*imnorm[pix] )

    where imnorm is image/sum(image) and gtot[pix] is
        
        gtot[pix] = sum_gi(gi[pix]) + nsky

    and nsky is the sky/sum(image)
    
    These new p[gi] can then be used to update the mean and covariance
    as well.  To update the mean in coordinate x

        mx[gi] = sum_pix( gi[pix]/gtot[pix]*imnorm[pix]*x )/pnew[gi]
 
    where x is the pixel value in either row or column.

    Similarly for the covariance for coord x and coord y.

        cxy = sum_pix(  gi[pix]/gtot[pix]*imnorm[pix]*(x-xc)*(y-yc) )/pcen[gi]

    setting x==y gives the diagonal terms.

    Then repeat until some tolerance in the moments is achieved.

    These calculations can be done very efficiently within a single loop,
    with a pixel lookup only once per loop.
    
 */
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "gmix_image.h"
#include "image.h"
#include "gvec.h"
#include "defs.h"

/*
 * This function is way too long and we put everything on the stack. A worry
 * if many gaussians, or linked to python?  There is a malloc one also and
 * it appears to be just as fast for a few gaussians.
 */

int gmix_image(struct gmix* self,
               struct image *image, 
               struct gvec *gvec,
               size_t *iter)
{
    int flags=0;
    size_t i=0;
    size_t ngauss = gvec->size;
    size_t nbytes = gvec->size*sizeof(double);
    double det=0, chi2=0, b=0;
    double u=0, v=0, uv=0, u2=0, v2=0, igrat=0;
    double gtot=0, imnorm=0, skysum=0.0, tau=0;
    double wmomlast=0, wmom=0, wmomdiff=0, psum=0;

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

    wmomlast=-9999;
    *iter=0;
    while (*iter < self->maxiter) {
        if (self->verbose)
            gvec_print(stderr,gvec);

        skysum=0;
        psum=0;
        memset(pnew,0,nbytes);
        memset(rowsum,0,nbytes);
        memset(colsum,0,nbytes);
        memset(u2sum,0,nbytes);
        memset(uvsum,0,nbytes);
        memset(v2sum,0,nbytes);

        for (size_t col=0; col<image->ncols; col++) {
            for (size_t row=0; row<image->nrows; row++) {

                imnorm=IMGET(image,row,col);
                imnorm /= counts;

                gtot=0;
                gauss = &gvec->data[0];
                for (i=0; i<ngauss; i++) {
                    det = gauss->irr*gauss->icc - gauss->irc*gauss->irc;
                    det = fabs(det);
                    if (det == 0) {
                        flags+=GMIX_ERROR_NEGATIVE_DET;
                        goto _gmix_image_bail;
                    }
                    u = (row-gauss->row);
                    v = (col-gauss->col);

                    u2 = u*u;
                    v2 = v*v;
                    uv = u*v;

                    chi2=gauss->icc*u2 + gauss->irr*v2 - 2.0*gauss->irc*uv;
                    chi2 /= det;
                    b = M_TWO_PI*sqrt(det);

                    gi[i] = gauss->p*exp( -0.5*chi2 )/b;
                    gtot += gi[i];

                    trowsum[i] = row*gi[i];
                    tcolsum[i] = col*gi[i];
                    tu2sum[i]  = u2*gi[i];
                    tuvsum[i]  = uv*gi[i];
                    tv2sum[i]  = v2*gi[i];

                    gauss++;
                }
                gtot += nsky;
                igrat = imnorm/gtot;
                for (i=0; i<ngauss; i++) {
                    tau = gi[i]*igrat;  // Dave's tau*imnorm
                    pnew[i] += tau;
                    psum += tau;

                    rowsum[i] += trowsum[i]*igrat;
                    colsum[i] += tcolsum[i]*igrat;
                    u2sum[i]  += tu2sum[i]*igrat;
                    uvsum[i]  += tuvsum[i]*igrat;
                    v2sum[i]  += tv2sum[i]*igrat;
                }
                skysum += nsky*imnorm/gtot;

            } // rows
        } // cols

        wmom=0;
        gauss=gvec->data;
        for (i=0; i<ngauss; i++) {
            gauss->p   = pnew[i];
            gauss->row = rowsum[i]/pnew[i];
            gauss->col = colsum[i]/pnew[i];
            gauss->irr = u2sum[i]/pnew[i];
            gauss->irc = uvsum[i]/pnew[i];
            gauss->icc = v2sum[i]/pnew[i];
            wmom += gauss->p*gauss->irr + gauss->p*gauss->icc;

            gauss++;
        }
        psky = skysum;

        wmom /= psum;
        wmomdiff = fabs(wmom-wmomlast);
        if (wmomdiff/wmom < self->tol) {
            break;
        }
        wmomlast = wmom;
        (*iter)++;
    }

_gmix_image_bail:
    if (self->maxiter == (*iter)) {
        flags += GMIX_ERROR_MAXIT;
    }
    return flags;
}


int gmix_image_malloc(struct gmix* self,
               struct image *image, 
               struct gvec *gvec,
               size_t *iter)
{
    int flags=0;
    size_t i=0;
    size_t ngauss = gvec->size;
    size_t nbytes = gvec->size*sizeof(double);
    double det=0, chi2=0, b=0;
    double u=0, v=0, uv=0, u2=0, v2=0, igrat=0;
    double gtot=0, imnorm=0, skysum=0.0, tau=0;
    double wmomlast=0, wmom=0, wmomdiff=0, psum=0;

    struct gauss* gauss=NULL;

    double sky=image_sky(image);
    double counts=image_counts(image);
    size_t npoints = IMSIZE(image);

    double nsky = sky/counts;
    double psky = sky/(counts/npoints);

    double *gi = calloc(1,nbytes);
    double *trowsum = calloc(1,nbytes);
    double *tcolsum = calloc(1,nbytes);
    double *tu2sum = calloc(1,nbytes);
    double *tuvsum = calloc(1,nbytes);
    double *tv2sum = calloc(1,nbytes);

    // these need to be zeroed on each iteration
    double *pnew = calloc(1,nbytes);
    double *rowsum = calloc(1,nbytes);
    double *colsum = calloc(1,nbytes);
    double *u2sum = calloc(1,nbytes);
    double *uvsum = calloc(1,nbytes);
    double *v2sum = calloc(1,nbytes);

    wmomlast=-9999;
    *iter=0;
    while (*iter < self->maxiter) {
        if (self->verbose)
            gvec_print(stderr,gvec);

        skysum=0;
        psum=0;
        memset(pnew,0,nbytes);
        memset(rowsum,0,nbytes);
        memset(colsum,0,nbytes);
        memset(u2sum,0,nbytes);
        memset(uvsum,0,nbytes);
        memset(v2sum,0,nbytes);

        for (size_t col=0; col<image->ncols; col++) {
            for (size_t row=0; row<image->nrows; row++) {

                imnorm=IMGET(image,row,col);
                imnorm /= counts;

                gtot=0;
                gauss = &gvec->data[0];
                for (i=0; i<ngauss; i++) {
                    det = gauss->irr*gauss->icc - gauss->irc*gauss->irc;
                    det = fabs(det);
                    if (det == 0) {
                        flags+=GMIX_ERROR_NEGATIVE_DET;
                        goto _gmix_image_bail;
                    }
                    u = (row-gauss->row);
                    v = (col-gauss->col);

                    u2 = u*u;
                    v2 = v*v;
                    uv = u*v;

                    chi2=gauss->icc*u2 + gauss->irr*v2 - 2.0*gauss->irc*uv;
                    chi2 /= det;
                    b = M_TWO_PI*sqrt(det);

                    gi[i] = gauss->p*exp( -0.5*chi2 )/b;
                    gtot += gi[i];

                    trowsum[i] = row*gi[i];
                    tcolsum[i] = col*gi[i];
                    tu2sum[i]  = u2*gi[i];
                    tuvsum[i]  = uv*gi[i];
                    tv2sum[i]  = v2*gi[i];

                    gauss++;
                }
                gtot += nsky;
                igrat = imnorm/gtot;
                for (i=0; i<ngauss; i++) {
                    tau = gi[i]*igrat;  // Dave's tau*imnorm
                    pnew[i] += tau;
                    psum += tau;

                    rowsum[i] += trowsum[i]*igrat;
                    colsum[i] += tcolsum[i]*igrat;
                    u2sum[i]  += tu2sum[i]*igrat;
                    uvsum[i]  += tuvsum[i]*igrat;
                    v2sum[i]  += tv2sum[i]*igrat;
                }
                skysum += nsky*imnorm/gtot;

            } // rows
        } // cols

        wmom=0;
        gauss=gvec->data;
        for (i=0; i<ngauss; i++) {
            gauss->p   = pnew[i];
            gauss->row = rowsum[i]/pnew[i];
            gauss->col = colsum[i]/pnew[i];
            gauss->irr = u2sum[i]/pnew[i];
            gauss->irc = uvsum[i]/pnew[i];
            gauss->icc = v2sum[i]/pnew[i];
            wmom += gauss->p*gauss->irr + gauss->p*gauss->icc;

            gauss++;
        }
        psky = skysum;

        wmom /= psum;
        wmomdiff = fabs(wmom-wmomlast);
        if (wmomdiff/wmom < self->tol) {
            break;
        }
        wmomlast = wmom;
        (*iter)++;
    }

_gmix_image_bail:
    if (self->maxiter == (*iter)) {
        flags += GMIX_ERROR_MAXIT;
    }

    free(gi);
    free(trowsum);
    free(tcolsum);
    free(tu2sum);
    free(tuvsum);
    free(tv2sum);
    free(pnew);
    free(rowsum);
    free(colsum);
    free(u2sum);
    free(uvsum);
    free(v2sum);
    return flags;
}
