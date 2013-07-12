#include <stdio.h>
#include <math.h>
#include <float.h> // for DBL_MAX
#include "gmix_image.h"
#include "image.h"
#include "gauss2.h"
#include "gmix.h"
#include "jacobian.h"

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
        flags |= GAUSS2_ERROR_NEGATIVE_DET;
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

    struct gauss2 *gauss=gmix->data;

    for (int i=0; i<gmix->size; i++) {
        gauss->row -= mask->rowmin;
        gauss->col -= mask->colmin;
        gauss++;
    }

    flags=gmix_image_put(masked_image,
                         gmix,
                         nsub);

    masked_image=image_free(masked_image);

    return flags;
}


static int get_loglike_wt_jacob_generic(const struct image *image, 
                                        const struct image *weight, // either
                                        double ivar,                // or
                                        const struct jacobian *jacob,
                                        const struct gmix *gmix, 
                                        double *s2n_numer,
                                        double *s2n_denom,
                                        double *loglike)
{
    size_t nrows=IM_NROWS(image), ncols=IM_NCOLS(image);

    double u=0, v=0;
    double diff=0;
    ssize_t col=0, row=0;

    double model_val=0;
    double pixval=0;
    int flags=0;

    (*s2n_numer)=0;
    (*s2n_denom)=0;
    (*loglike)=0;

    if (!gmix_verify(gmix)) {
        *loglike=-9999.9e9;
        flags |= GAUSS2_ERROR_NEGATIVE_DET;
        goto _calculate_loglike_wt_jacob_bail;
    }

    if (ivar < 0) ivar=0.0;
    for (row=0; row<nrows; row++) {
        u=JACOB_PIX2U(jacob, row, 0);
        v=JACOB_PIX2V(jacob, row, 0);
        for (col=0; col<ncols; col++) {

            if (weight) {
                ivar=IM_GET(weight, row, col);
                if (ivar < 0) ivar=0.0; // fpack...
            }

            if (ivar > 0) {
                model_val=GMIX_EVAL(gmix, u, v);
                pixval=IM_GET(image, row, col);
                diff = model_val - pixval;

                (*loglike) += diff*diff*ivar;

                (*s2n_numer) += pixval*model_val*ivar;
                (*s2n_denom) += model_val*model_val*ivar;
            }

            u += jacob->dudcol; v += jacob->dvdcol;
        } // cols
    } // rows

    (*loglike) *= (-0.5);

_calculate_loglike_wt_jacob_bail:
    return flags;
}

int gmix_image_loglike_wt_jacob(const struct image *image, 
                                const struct image *weight,
                                const struct jacobian *jacob,
                                const struct gmix *gmix, 
                                double *s2n_numer,
                                double *s2n_denom,
                                double *loglike)

{

    double junk_ivar=0;
    return get_loglike_wt_jacob_generic(image, 
                                        weight,
                                        junk_ivar,
                                        jacob,
                                        gmix, 
                                        s2n_numer,
                                        s2n_denom,
                                        loglike);

}


int gmix_image_loglike_ivar_jacob(const struct image *image, 
                                  double ivar,
                                  const struct jacobian *jacob,
                                  const struct gmix *gmix, 
                                  double *s2n_numer,
                                  double *s2n_denom,
                                  double *loglike)
{

    struct image *junk_weight=NULL;

    return get_loglike_wt_jacob_generic(image, 
                                        junk_weight,
                                        ivar,
                                        jacob,
                                        gmix, 
                                        s2n_numer,
                                        s2n_denom,
                                        loglike);

}

int gmix_image_loglike_wt(const struct image *image, 
                          const struct image *weight,
                          const struct gmix *gmix, 
                          double *s2n_numer,
                          double *s2n_denom,
                          double *loglike)
{

    double junk_ivar=0;
    struct jacobian jacob;
    jacobian_set_identity(&jacob);

    return get_loglike_wt_jacob_generic(image, 
                                        weight,
                                        junk_ivar,
                                        &jacob,
                                        gmix, 
                                        s2n_numer,
                                        s2n_denom,
                                        loglike);

}


int gmix_image_loglike_ivar(const struct image *image, 
                            const struct gmix *gmix, 
                            double ivar,
                            double *s2n_numer,
                            double *s2n_denom,
                            double *loglike)
{

    int flags=0;
    struct image *junk_weight=NULL;

    struct jacobian jacob;
    jacobian_set_identity(&jacob);

    flags=get_loglike_wt_jacob_generic(image, 
                                       junk_weight,
                                       ivar,
                                       &jacob,
                                       gmix, 
                                       s2n_numer,
                                       s2n_denom,
                                       loglike);


    return flags;

}

double gmix_image_s2n_ivar(const struct image *image, 
                           const struct gmix *weight,
                           double ivar, 
                           long *flags)
{
    double s2n=-9999;
    double s2n_numer=0, s2n_denom=0, loglike=0;
    *flags=gmix_image_loglike_ivar(image, 
                                   weight, 
                                   ivar,
                                   &s2n_numer,
                                   &s2n_denom,
                                   &loglike);

    if (*flags == 0) {
        if (s2n_denom >= 0) {
            s2n = s2n_numer/sqrt(s2n_denom);
        }
    }

    return s2n;
}



// If this image is masked, the gaussian should be centered
// properly in the sub-image, not the parent
/*
double gmix_image_loglike(const struct image *image, 
                          const struct gmix *gmix, 
                          double ivar,
                          int *flags)
{
    size_t nrows=IM_NROWS(image), ncols=IM_NCOLS(image);

    //struct gauss2 *gauss=NULL;
    double diff=0;
    size_t col=0, row=0;

    double loglike = 0;
    double model_val=0;
    double *rowdata=NULL;


    (*flags)=0;

    if (!gmix_verify(gmix)) {
        (*flags) |= GAUSS2_ERROR_NEGATIVE_DET;
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
*/

// should dump this, only works with no jacob
/*
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
        (*flags) |= GAUSS2_ERROR_NEGATIVE_DET;
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
*/
