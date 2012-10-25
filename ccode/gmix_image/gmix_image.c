#include <stdio.h>
#include <math.h>
#include <float.h> // for DBL_MAX
#include "gmix_image.h"
#include "image.h"
#include "gauss.h"
#include "gmix.h"
#include "randn.h"

struct image *gmix_image_new(const struct gmix *gmix, 
                             size_t nrows, 
                             size_t ncols, 
                             int nsub)
{
    struct image *im = image_new(nrows, ncols);
    gmix_image_fill(im, gmix, nsub);
    return im;
}

int gmix_image_fill(struct image *image, 
                    const struct gmix *gmix, 
                    int nsub)
{
    size_t nrows=IM_NROWS(image), ncols=IM_NCOLS(image);

    struct gauss *gauss=NULL;
    double u=0, v=0;
    double chi2=0;//, b=0;
    size_t i=0, col=0, row=0, irowsub=0, icolsub=0;
    double onebynsub2 = 1./(nsub*nsub);
    double counts=0;

    double model_val=0, tval=0;
    double stepsize=0, offset=0, trow=0, tcol=0;
    int flags=0;

    if (!gmix_verify(gmix)) {
        flags |= GMIX_IMAGE_NEGATIVE_DET;
        goto _gmix_image_fill_model_bail;
    }
    if (nsub < 1) {
        nsub=1;
    }
    stepsize = 1./nsub;
    offset = (nsub-1)*stepsize/2.;

    for (row=0; row<nrows; row++) {
        for (col=0; col<ncols; col++) {

            model_val=0;
            gauss=gmix->data;
            for (i=0; i<gmix->size; i++) {

                // work over the subgrid
                tval=0;
                trow = row-offset;
                for (irowsub=0; irowsub<nsub; irowsub++) {
                    tcol = col-offset;
                    for (icolsub=0; icolsub<nsub; icolsub++) {

                        u = trow-gauss->row;
                        v = tcol-gauss->col;

                        chi2=gauss->dcc*u*u + gauss->drr*v*v - 2.0*gauss->drc*u*v;
                        tval += gauss->norm*gauss->p*exp( -0.5*chi2 );

                        tcol += stepsize;
                    }
                    trow += stepsize;
                }

                //b = M_TWO_PI*sqrt(gauss->det);
                //tval /= (b*nsub*nsub);
                tval *= onebynsub2;
                model_val += tval;

                gauss++;
            } // gmix

            IM_SETFAST(image, row, col, model_val);
            counts += model_val;

        } // cols
    } // rows

    IM_SET_COUNTS(image, counts);

_gmix_image_fill_model_bail:
    return flags;
}

double gmix_image_loglike(const struct image *image, 
                          const struct gmix *gmix, 
                          double ivar,
                          int *flags)
{
    size_t nrows=IM_NROWS(image), ncols=IM_NCOLS(image);

    struct gauss *gauss=NULL;
    double u=0, v=0;
    double chi2=0, diff=0;
    size_t i=0, col=0, row=0;

    double loglike = 0;
    double model_val=0;
    double *rowdata=NULL;

    (*flags)=0;

    if (!gmix_verify(gmix)) {
        (*flags) |= GMIX_IMAGE_NEGATIVE_DET;
        loglike = GMIX_IMAGE_LOW_VAL;
        goto _gmix_image_loglike_bail;
    }


    for (row=0; row<nrows; row++) {
        rowdata=IM_ROW(image, row);
        for (col=0; col<ncols; col++) {

            model_val=0;
            gauss=gmix->data;
            for (i=0; i<gmix->size; i++) {
                u = row-gauss->row;
                v = col-gauss->col;

                chi2=gauss->dcc*u*u + gauss->drr*v*v - 2.0*gauss->drc*u*v;
                model_val += gauss->norm*gauss->p*exp( -0.5*chi2 );

                gauss++;
            } // gmix

            diff = model_val -(*rowdata);
            loglike += diff*diff*ivar;

            rowdata++;
        } // cols
    } // rows

    loglike *= (-0.5);
    //fprintf(stderr,"loglike: %.16g\n", loglike);
    //exit(1);

_gmix_image_loglike_bail:

    return loglike;
}

double gmix_image_loglike_margamp(
        const struct image *image, 
        const struct gmix *gmix, 
        double ivar,
        int *flags)
{
    size_t nrows=IM_NROWS(image), ncols=IM_NCOLS(image);

    struct gauss *gauss=NULL;
    double u=0, v=0;
    double chi2=0;
    size_t i=0, col=0, row=0;

    double loglike = 0;
    double model_val=0;
    double *rowdata=NULL;

    double ierr=sqrt(ivar);
    double A=1;
    double ymodsum=0; // sum of (image/err)
    double ymod2sum=0; // sum of (image/err)^2
    double B=0.; // sum(model*image/err^2)/A

    (*flags)=0;

    if (!gmix_verify(gmix)) {
        (*flags) |= GMIX_IMAGE_NEGATIVE_DET;
        loglike = GMIX_IMAGE_LOW_VAL;
        goto _gmix_image_loglike_margamp_bail;
    }


    for (row=0; row<nrows; row++) {
        rowdata=IM_ROW(image, row);
        for (col=0; col<ncols; col++) {

            model_val=0;
            gauss=gmix->data;
            for (i=0; i<gmix->size; i++) {
                u = row-gauss->row;
                v = col-gauss->col;

                chi2=gauss->dcc*u*u + gauss->drr*v*v - 2.0*gauss->drc*u*v;
                model_val += gauss->norm*gauss->p*exp( -0.5*chi2 );

                gauss++;
            } // gmix

            ymodsum += model_val;
            ymod2sum += model_val*model_val;
            B += (*rowdata)*model_val;

            rowdata++;
        } // cols
    } // rows


    ymodsum *= ierr;
    ymod2sum *= ierr*ierr;
    double norm = sqrt(ymodsum*ymodsum*A/ymod2sum);

    // renorm so A is fixed; also extra factor of 1/err^2 and 1/A
    B *= (norm/ymodsum*ierr*ierr/A);

    loglike = 0.5*A*B*B;


_gmix_image_loglike_margamp_bail:

    return loglike;
}




double gmix_image_s2n(const struct image *image, 
                      double skysig, 
                      const struct gmix *weight,
                      int *flags)
{
    size_t nrows=IM_NROWS(image), ncols=IM_NCOLS(image);

    struct gauss *gauss=NULL;
    size_t i=0, col=0, row=0;
    double sum=0, wsum=0, wt=0, *rowdata=NULL;
    double s2n=-9999;

    (*flags)=0;

    if (!gmix_verify(weight)) {
        (*flags) |= GMIX_IMAGE_NEGATIVE_DET;
        goto _gmix_image_s2n_noise_bail;
    }

    for (row=0; row<nrows; row++) {
        rowdata=IM_ROW(image, row);
        for (col=0; col<ncols; col++) {

            wt=0;
            gauss=weight->data;
            for (i=0; i<weight->size; i++) {
                wt += GAUSS_EVAL(gauss,row,col);
                gauss++;
            } // gmix

            sum += (*rowdata)*wt;
            wsum += wt;

            rowdata++;
        } // cols
    } // rows

    s2n = sum/sqrt(wsum)/skysig;

_gmix_image_s2n_noise_bail:
    return s2n;

}

static void _image_add_noise(struct image *image, double skysig)
{
    size_t nrows=IM_NROWS(image), ncols=IM_NCOLS(image);

    size_t col=0, row=0;
    double *rowdata=NULL;

    for (row=0; row<nrows; row++) {
        rowdata=IM_ROW(image, row);
        for (col=0; col<ncols; col++) {
            (*rowdata) += skysig*randn();
            rowdata++;
        } // cols
    } // rows
}

int gmix_image_add_noise(struct image *image, 
                         double s2n,
                         const struct gmix *gmix,
                         double *skysig) 
{
    int flags=0;

    if (!gmix_verify(gmix)) {
        flags |= GMIX_IMAGE_NEGATIVE_DET;
        goto _gmix_image_add_noise_bail;
    }

    double tskysig=1;
    double s2n_first_pass=gmix_image_s2n(image,tskysig,gmix,&flags);
    if (flags!=0) {
        goto _gmix_image_add_noise_bail;
    }
    (*skysig) = s2n_first_pass/s2n * tskysig;

    _image_add_noise(image, (*skysig));

_gmix_image_add_noise_bail:
    return flags;
}


