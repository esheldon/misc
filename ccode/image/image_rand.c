#include "randn.h"
#include "image.h"
#include "image_rand.h"

void image_add_randn(struct image *image, double skysig)
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


