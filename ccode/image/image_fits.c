#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fitsio.h>
#include "image.h"

#define CFITSIO_MAX_ARRAY_DIMS 99

static void print_error_and_exit(int status)
{
    char status_str[FLEN_STATUS];
    fits_get_errstatus(status, status_str);  /* get the error description */
    fprintf(stderr,"FITSIO error = %d: %s\n", status, status_str);
    exit(EXIT_FAILURE);
}

/* 
   add a ! to front of name so cfitsio will clobber any existing file 
   you must free the returned string.
*/
static char *get_clobber_name(const char *filename)
{
    char *oname=NULL;
    int len=strlen(filename);

    oname = calloc(len+2, sizeof(char));
    oname[0]='!';

    strncpy(oname+1, filename, len);
    return oname;
}


void image_write_fits(const struct image *self,
                      const char *filename,
                      int clobber)
{
    fitsfile* fits=NULL;
    LONGLONG firstpixel=1;
    LONGLONG nelements=0;

    int ndims=2;
    long dims[2]={0};

    char *name=NULL;

    if (clobber) {
        name=get_clobber_name(filename);
    } else {
        name=strdup(filename);
    }

    int status=0;
    if (fits_create_file(&fits, name, &status)) {
        print_error_and_exit(status);
    }

    dims[1] = IM_NROWS(self);
    dims[0] = IM_NCOLS(self);
    if (fits_create_img(fits, DOUBLE_IMG, ndims, dims, &status)) {
        print_error_and_exit(status);
    }

    nelements=IM_SIZE(self);
    if (fits_write_img(fits, TDOUBLE, firstpixel, nelements, 
                       IM_GETP(self,0,0), &status)) {
        print_error_and_exit(status);
    }

    if (fits_close_file(fits, &status)) {
        print_error_and_exit(status);
    }

    free(name);
}

struct image *image_read_fits(const char *fname, int ext)
{
    fitsfile* fits=NULL;
    struct image *image=NULL;

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
    //fprintf(stderr,"dims: [%lld,%lld]\n", dims[0], dims[1]);

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
