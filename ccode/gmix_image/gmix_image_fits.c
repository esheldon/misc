#include <stdlib.h>
#include <stdio.h>
#include <fitsio.h>
#include "image.h"

#define CFITSIO_MAX_ARRAY_DIMS 99

struct image *image_read_fits(const char *fname, int ext)
{
    fitsfile* fits=NULL;
    struct image *image=NULL;

    fprintf(stderr,"reading ext %d from %s\n",ext,fname);

    int status=0;
    if (fits_open_file(&fits, fname, READONLY, &status)) {
        fits_report_error(stderr, status);
        exit(EXIT_FAILURE);
    }

    int hdutype=0;
    if (fits_movabs_hdu(fits, ext+1, &hdutype, &status)) {
        fits_report_error(stderr, status);
        exit(EXIT_FAILURE);
    }


    int maxdim=CFITSIO_MAX_ARRAY_DIMS;
    LONGLONG dims[CFITSIO_MAX_ARRAY_DIMS];
    int bitpix=0, ndims=0;
    if (fits_get_img_paramll(fits, maxdim, &bitpix, &ndims, dims, &status)) {
        fits_report_error(stderr, status);
        exit(EXIT_FAILURE);
    }
    if (ndims != 2) {
        fprintf(stderr,"expected ndims=2, got %d\n", ndims);
        goto _test_read_bail;
    }
    // dims reversed
    fprintf(stderr,"dims: [%lld,%lld]\n", dims[0], dims[1]);

    // note dims are reversed
    image=image_new(dims[1], dims[0]);
    long npix=dims[1]*dims[0];
    long fpixel[2]={1,1};
    if (fits_read_pix(fits, TDOUBLE, fpixel, npix,
                      NULL,image->rows[0], NULL, &status)) {
        fits_report_error(stderr, status);
        exit(EXIT_FAILURE);
    }

_test_read_bail:
    if (status) {
        image=image_free(image);
    }
    if (fits_close_file(fits, &status)) {
        fits_report_error(stderr, status);
        exit(EXIT_FAILURE);
    }

    return image;
}
