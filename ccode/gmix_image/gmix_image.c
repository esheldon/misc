#include <stdio.h>
#include <math.h>
#include <float.h> // for DBL_MAX
#include "gmix_image.h"
#include "image.h"
#include "gauss.h"
#include "gmix.h"
#include "rand.h"

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

            //fprintf(stderr,"Model val: %.16g rowdata: %.16g\n", model_val, (*rowdata));
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

int gmix_image_add_noise(struct image *image, 
                         double s2n,
                         const struct gmix *gmix,
                         double *skysig, 
                         double *s2n_meas)
{
    size_t nrows=IM_NROWS(image), ncols=IM_NCOLS(image);

    struct gauss *gauss=NULL;
    double u=0, v=0;
    double chi2=0;
    size_t i=0, col=0, row=0;
    double sum=0, wsum=0, wt=0;

    if (!gmix_verify(gmix)) {
        flags |= GMIX_IMAGE_NEGATIVE_DET;
        goto _gmix_image_add_noise_bail;
    }

    for (pass=1;pass<=2;pass++) {
        for (row=0; row<nrows; row++) {
            rowdata=IM_ROW(image, row);
            for (col=0; col<ncols; col++) {

                if (pass==1) {
                    wt=0;
                    gauss=gmix->data;
                    for (i=0; i<gmix->size; i++) {

                        u = row-gauss->row;
                        v = col-gauss->col;

                        chi2 =
                            gauss->dcc*u*u 
                            + gauss->drr*v*v 
                            - 2.0*gauss->drc*u*v;

                        wt +=  gauss->norm*gauss->p*exp( -0.5*chi2 );

                        gauss++;
                    } // gmix

                    sum += (*rowdata)*wt;
                    wsum += wt;

                } else  {
                    (*rowdata) += (*skysig) * randn();
                }
                rowdata++;
            } // cols
        } // rows

        if (pass==1) {
            // this new skysig should give us the requested S/N
            (*skysig) = (*s2n_meas)/s2n * (*skysig);
        } else {
            (*s2n_meas) = sum/sqrt(wsum)/(*skysig);
        }

    }


_gmix_image_add_noise_bail:
    return flags;
}


