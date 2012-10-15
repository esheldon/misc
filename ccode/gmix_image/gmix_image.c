#include <math.h>
#include "gmix_image.h"
#include "image.h"
#include "gauss.h"
#include "gmix.h"

struct image *gmix_image_new(struct gmix *gmix, size_t nrows, size_t ncols, int nsub)
{
    struct image *im = image_new(nrows, ncols);
    gmix_image_fill(im, gmix, nsub);
    return im;
}

int gmix_image_fill(struct image *image, 
                    struct gmix *gmix, 
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


