#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "gmix_image.h"
#include "image.h"
#include "gvec.h"
#include "defs.h"

int gmix_image(struct gmix* self,
               struct image *image, 
               struct gvec *gvec,
               size_t *iter)
{
    int flags=0;
    size_t i=0;

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

    double *pnew = alloca(gvec->size);
    double *gi = alloca(gvec->size);
    double *rowsum = alloca(gvec->size);
    double *trowsum = alloca(gvec->size);
    double *colsum = alloca(gvec->size);
    double *tcolsum = alloca(gvec->size);

    double *tu2sum = alloca(gvec->size);
    double *u2sum = alloca(gvec->size);
    double *tuvsum = alloca(gvec->size);
    double *uvsum = alloca(gvec->size);
    double *tv2sum = alloca(gvec->size);
    double *v2sum = alloca(gvec->size);


    wmomlast=-9999;
    *iter=0;
    while (*iter < self->maxiter) {

        skysum=0;
        psum=0;
        memset(pnew,0,gvec->size*sizeof(double));
        memset(rowsum,0,gvec->size*sizeof(double));
        memset(colsum,0,gvec->size*sizeof(double));
        memset(u2sum,0,gvec->size*sizeof(double));
        memset(uvsum,0,gvec->size*sizeof(double));
        memset(v2sum,0,gvec->size*sizeof(double));

        for (size_t col=0; col<image->ncols; col++) {
            for (size_t row=0; row<image->nrows; row++) {

                imnorm=IMGET(image,row,col);
                imnorm /= counts;

                gtot=0;
                gauss = &gvec->data[0];
                for (i=0; i<gvec->size; i++) {
                    det = gauss->irr*gauss->icc - gauss->irc*gauss->irc;
                    det = fabs(det);
                    if (det == 0) {
                        wlog("det == 0\n");
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

                    tu2sum[i] = u2*gi[i];
                    tuvsum[i] = uv*gi[i];
                    tv2sum[i] = v2*gi[i];

                    gauss++;
                }
                gtot += nsky;
                for (i=0; i<gvec->size; i++) {
                    igrat = imnorm/gtot;
                    tau = gi[i]*igrat;  // Dave's tau*imnorm
                    pnew[i] += tau;
                    psum += tau;

                    rowsum[i] += trowsum[i]*igrat;
                    colsum[i] += tcolsum[i]*igrat;
                    u2sum[i] += tu2sum[i]*igrat;
                    uvsum[i] += tuvsum[i]*igrat;
                    v2sum[i] += tv2sum[i]*igrat;
                }
                skysum += nsky*imnorm/gtot;

            } // rows
        } // cols

        wmom=0;
        gauss=gvec->data;
        for (i=0; i<gvec->size; i++) {
            gauss->p = pnew[i];
            gauss->row = rowsum[i]/pnew[i];
            gauss->col = colsum[i]/pnew[i];
            gauss->irr = u2sum[i]/pnew[i];
            gauss->irc = uvsum[i]/pnew[i];
            gauss->icc = v2sum[i]/pnew[i];

            wmom += gauss->p*gauss->irr + gauss->p*gauss->icc;
            if (self->verbose) {
                wlog("gauss[%lu] pnew: %.16g row: %.16g col: %.16g " 
                     "irr: %.16g irc: %.16g icc: %.16g\n",
                     i, gauss->p, gauss->row, gauss->col,
                     gauss->irr,gauss->irc, gauss->icc);
            }
           gauss++;
        }
        psky = skysum;
        if (self->verbose)
            wlog("psky: %.16g\n", psky);

        wmom /= psum;
        if (self->verbose)
            wlog("wmom: %.16g\n", wmom);
        wmomdiff = fabs(wmom-wmomlast);
        if (wmomdiff/wmom < self->tol) {
        //if (wmomdiff < self->tol) {
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
