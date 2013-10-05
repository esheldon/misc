#include <math.h>
#include "randn.h"
#include "image.h"
#include "image_rand.h"

void image_add_randn(struct image *image, double skysig)
{
    size_t nrows=IM_NROWS(image), ncols=IM_NCOLS(image);

    size_t col=0, row=0;
    double *rowdata=NULL;

    if (skysig <= 0) {
        return;
    }

    for (row=0; row<nrows; row++) {
        rowdata=IM_ROW(image, row);
        for (col=0; col<ncols; col++) {
            (*rowdata) += skysig*randn();
            rowdata++;
        } // cols
    } // rows
}

void image_add_randn_matched(struct image *image, double s2n, double *skysig)
{
    size_t size=IM_SIZE(image);
    double *rowdata=NULL, sum2=0;

    for (long pass=1; pass <= 2; pass++) {
        rowdata=IM_ROW(image, 0);
        for (size_t i=0; i<size; i++) {
            if (pass==1) {
                sum2 += (*rowdata)*(*rowdata);
            } else {
                (*rowdata) += (*skysig)*randn();
            }
            rowdata++;
        }
        if (pass==1) {
            *skysig = sqrt(sum2)/s2n;
        }
    } // passes
}


void image_add_poisson(struct image *image)
{
    size_t nrows=IM_NROWS(image), ncols=IM_NCOLS(image);

    size_t col=0, row=0;
    double *rowdata=NULL;

    for (row=0; row<nrows; row++) {
        rowdata=IM_ROW(image, row);
        for (col=0; col<ncols; col++) {

            double val=(*rowdata);
            long pval = poisson(val);
            (*rowdata) = (double) pval;
            rowdata++;

        } // cols
    } // rows
}


