#include <stdio.h>
#include <math.h>
#include <float.h> // for DBL_MAX
#include "gmix_image.h"
#include "image.h"
#include "gauss.h"
#include "gmix.h"
#include "fmath.h"

struct image *gmix_image_new(const struct gmix *gmix, 
                             size_t nrows, 
                             size_t ncols, 
                             int nsub)
{
    struct image *im = image_new(nrows, ncols);
    gmix_image_put(im, gmix, nsub);
    return im;
}

/*
   Add the gaussian mixture to the image.

   The values are *added* so be sure to initialize
   properly.
*/
int gmix_image_put(struct image *image, 
                   const struct gmix *gmix, 
                   int nsub)
{
    size_t nrows=IM_NROWS(image), ncols=IM_NCOLS(image);

    size_t col=0, row=0, irowsub=0, icolsub=0;
    double onebynsub2 = 1./(nsub*nsub);
    double counts=0;

    double model_val=0, tval=0;
    double stepsize=0, offset=0, trow=0, tcol=0;
    int flags=0;

    if (!gmix_verify(gmix)) {
        flags |= GMIX_IMAGE_NEGATIVE_DET;
        goto _gmix_image_put_model_bail;
    }
    if (nsub < 1) {
        nsub=1;
    }
    stepsize = 1./nsub;
    offset = (nsub-1)*stepsize/2.;

    for (row=0; row<nrows; row++) {
        for (col=0; col<ncols; col++) {


            tval=0;
            trow = row-offset;
            for (irowsub=0; irowsub<nsub; irowsub++) {
                tcol = col-offset;
                for (icolsub=0; icolsub<nsub; icolsub++) {
                    tval += GMIX_EVAL(gmix, trow, tcol);
                    tcol += stepsize;
                }
                trow += stepsize;
            }

            tval *= onebynsub2;

            model_val=IM_GET(image, row, col);
            model_val += tval;

            IM_SETFAST(image, row, col, model_val);
            counts += model_val;

        } // cols
    } // rows

_gmix_image_put_model_bail:
    return flags;
}

int gmix_image_put_masked(struct image *image, 
                          const struct gmix *gmix, 
                          int nsub,
                          struct image_mask *mask)
{
    int flags=0;

    struct image *masked_image=image_get_ref(image);

    image_add_mask(masked_image, mask);

    struct gmix *masked_gmix = gmix_new_copy(gmix);

    struct gauss *gauss=masked_gmix->data;

    for (int i=0; i<gmix->size; i++) {
        gauss->row -= mask->rowmin;
        gauss->col -= mask->colmin;
        gauss++;
    }

    flags=gmix_image_put(masked_image,
                         gmix,
                         nsub);

    masked_image=image_free(masked_image);
    masked_gmix=gmix_free(masked_gmix);

    return flags;
}

// If this image is masked, the gaussian should be centered
// properly in the sub-image, not the parent
double gmix_image_loglike(const struct image *image, 
                          const struct gmix *gmix, 
                          double ivar,
                          int *flags)
{
    size_t nrows=IM_NROWS(image), ncols=IM_NCOLS(image);

    //struct gauss *gauss=NULL;
    double diff=0;
    size_t col=0, row=0;

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
        // always use IM_ROW incase the image is masked
        rowdata=IM_ROW(image, row);
        for (col=0; col<ncols; col++) {

            model_val = GMIX_EVAL(gmix, row, col);
            diff = model_val -(*rowdata);
            loglike += diff*diff*ivar;

            rowdata++;
        } // cols
    } // rows

    loglike *= (-0.5);

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

    size_t col=0, row=0;

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

            model_val = GMIX_EVAL(gmix, row, col);

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

    size_t col=0, row=0;
    double sum=0, w2sum=0, wt=0, *rowdata=NULL;
    double s2n=-9999;

    (*flags)=0;

    if (!gmix_verify(weight)) {
        (*flags) |= GMIX_IMAGE_NEGATIVE_DET;
        goto _gmix_image_s2n_noise_bail;
    }

    for (row=0; row<nrows; row++) {
        rowdata=IM_ROW(image, row);
        for (col=0; col<ncols; col++) {

            wt=GMIX_EVAL(weight, row, col);

            sum += (*rowdata)*wt;
            w2sum += wt*wt;

            rowdata++;
        } // cols
    } // rows

    s2n = sum/sqrt(w2sum)/skysig;

_gmix_image_s2n_noise_bail:
    return s2n;

}

