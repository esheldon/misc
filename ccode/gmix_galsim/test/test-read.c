#include <stdlib.h>
#include <stdio.h>
#include <fitsio.h>
#include "../image.h"

#define CFITSIO_MAX_ARRAY_DIMS 99

int main(int argc, char **argv)
{
    if (argc < 3) {
        fprintf(stderr,"test-read filename ext\n");
        exit(EXIT_FAILURE);
    }
    fitsfile* fits=NULL;
    struct image *image=NULL;

    const char *fname=argv[1];
    int ext = atoi(argv[2]);

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
    fprintf(stderr,"dims: [%lld,%lld]\n", dims[1], dims[2]);

    // note dims are reversed
    image=image_new(dims[1], dims[0]);
    long npix=dims[1]*dims[0];
    long fpixel[2]={1,1};
    if (fits_read_pix(fits, TDOUBLE, fpixel, npix,
                      NULL,image->rows[0], NULL, &status)) {
        fits_report_error(stderr, status);
        exit(EXIT_FAILURE);
    }

    size_t row=0,col=0;

    row=10; col=13;
    fprintf(stderr,"im[%ld,%ld]: %.16g\n",row,col,IM_GET(image,row,col));
    row=10; col=10;
    fprintf(stderr,"im[%ld,%ld]: %.16g\n",row,col,IM_GET(image,row,col));
    row=18; col=24;
    fprintf(stderr,"im[%ld,%ld]: %.16g\n",row,col,IM_GET(image,row,col));


_test_read_bail:
    image=image_free(image);
    if (fits_close_file(fits, &status)) {
        fits_report_error(stderr, status);
        exit(EXIT_FAILURE);
    }
}
