#include <string.h>
#include <stdio.h>
#include <stdlib.h>


#include "fitsio.h"
#include "fitsio2.h"


struct colinfo {
    int ncols;
    char** ttype;    /* column name */
    int*   datatype; /* integer data type code */
    long*  repeat;   /* for array columns, the total count (not shape) */
    long*  width;    /* number of bytes.  for string columns, the width of the string */
};

struct colinfo* colinfo_new(int ncols) {
    int i=0;
    struct colinfo* colinfo;

    colinfo = calloc(1, sizeof(struct colinfo));
    colinfo->ncols = ncols;

    colinfo->ttype = calloc(ncols, sizeof(char*));
    for (i=0; i<ncols; i++) {
        colinfo->ttype[i] = malloc(FLEN_VALUE);  /* max label length = 69 */
    }

    colinfo->datatype = calloc(ncols, sizeof(int));
    colinfo->repeat   = calloc(ncols, sizeof(long));
    colinfo->width    = calloc(ncols, sizeof(long));

    return colinfo;
}

struct colinfo* colinfo_delete(struct colinfo* colinfo) {
    int i=0;
    if (colinfo != NULL) {
        for (i=0; i<colinfo->ncols; i++) {
            free(colinfo->ttype[i]);
        }
        free(colinfo->ttype);
        free(colinfo->datatype);
        free(colinfo->repeat);
        free(colinfo->width);
        free(colinfo);

    }
    return NULL;
}



int main(int argc, char** argv) {
    const char* filename="test1.fits";

    // need some pre-processor stuff to match
    // up cfitsio types to numpy types.  on
    // linux this should work
    //   TLONG -> npy_int32
    //   TDOUBLE -> npy_float64
    // note ffgcv does int/long incorrectly on linux!

    int* index=NULL;
    double* x=NULL;
    double* y=NULL;

    fitsfile* fits=NULL;
    FITSfile* hdu=NULL;
    tcolumn* col=NULL;

    int status=0;
    int ncols=0;
    long nrows=0;

    int nfound=0, i=0;
    int hdunum, hdutype=0;
    int* hdutypes=NULL;

    int colnum;
    char cdummy[2];
    int anynul;

    int rows2read[] = {2,5,9};
    int nrows2read = sizeof(rows2read)/sizeof(int);

    int nhdu;

#if BYTESWAPPED
    printf("this platform is byteswapped\n");
#endif
    printf("sizeof(int): %lu\n", sizeof(int));
    printf("sizeof(long): %lu\n", sizeof(long));
    printf("opening: %s\n", filename);
    if (fits_open_file(&fits, filename, READONLY, &status) != 0) {
        fits_report_error(stderr, status);
        exit(EXIT_FAILURE);
    }

    // can use this routine, but we want to get some more info on
    // the fly, so we will roll our own
    /*
    if (fits_get_num_hdus(fits, &nhdu, &status)) {
        fits_report_error(stderr, status);
        exit(EXIT_FAILURE);
    }
    printf("Found %d HDUs\n", nhdu);
    */

    nhdu=0;
    hdutypes = malloc(sizeof(int));
    for (hdunum=1; hdunum<1000; hdunum++) {
        if (fits_movabs_hdu(fits, hdunum, &hdutype, &status)) {
            break;
        }
        nhdu += 1;
        hdutypes = realloc(hdutypes, nhdu*sizeof(int)); 
        hdutypes[nhdu-1] = hdutype;
    }
    printf("Found %d HDUs\n", nhdu);
    if (nhdu == 0) {
        return 0;
    }

    for (i=0; i<nhdu; i++) {
        if (hdutypes[i] == IMAGE_HDU) {
            printf("  HDU: %d  HDU type: IMAGE_HDU (%d)\n", i+1, hdutypes[i]);
        } else if (hdutypes[i] == BINARY_TBL) {
            printf("  HDU: %d  HDU type: BINARY_TBL (%d)\n", i+1, hdutypes[i]);
        } else if (hdutypes[i] == ASCII_TBL) {
            printf("  HDU: %d  HDU type: ASCII_TBL (%d)\n", i+1, hdutypes[i]);
        } else {
            printf("  HDU: %d  HDU type: UNKNOWN (%d)\n", i+1, hdutypes[i]);
        }
    }

    status=0;
    hdunum=2;
    printf("moving to hdu : %d\n", hdunum);
    if (fits_movabs_hdu(fits, hdunum, &hdutype, &status)!=0) {
        fits_report_error(stderr, status);
        exit(EXIT_FAILURE);
    }

    hdu = fits->Fptr;
    if (hdu->hdutype == IMAGE_HDU) {
        fprintf(stderr,"hdu is not a table");
        exit(EXIT_FAILURE);
    }

    printf("nrows: %lld\n", hdu->numrows);
    printf("ncols: %d\n", hdu->tfield);

    // point to first column
    col = hdu->tableptr;
    for (i=0; i<hdu->tfield; i++) {
        printf("TTYPE%d: %15s datatype: %d repeat: %lld width: %ld\n", 
               i+1, 
               col->ttype,
               col->tdatatype,
               col->trepeat,
               col->twidth);
        col++;
    }

    index = calloc(nrows2read, sizeof(int));
    index[0] = -9999;
    x = calloc(nrows2read, sizeof(double));
    y = calloc(nrows2read, sizeof(double));

    // read some data
    // first go to start of table

    //tbcol     = colptr->tbcol;   /* offset to start of column within row   */
    col = hdu->tableptr + 0;
    printf("datastart: %lld\n", hdu->datastart);
    printf("tbcol: %lld\n", col->tbcol);

    /*
    if (fits_read_col(fits, TINT, colnum, 1, 1, hdu->numrows, 0, (void*) index, NULL, &status)) {
        fits_report_error(stderr, status);
        exit(EXIT_FAILURE);
    }
    */
    /*
    if (ffgclk(fits, colnum, 1, 1, hdu->numrows, 1, 1, 0, index, cdummy, &anynul, &status)) {
        fits_report_error(stderr, status);
        exit(EXIT_FAILURE);
    }
    */

    //ffmbyt(fits, hdu->datastart + 0*hdu->rowlength + col->tbcol, REPORT_EOF, &status);
    //for (i=0; i<hdu->numrows; i++) {
    for (i=0; i<nrows2read; i++) {
        int row=rows2read[i];
        colnum = 1;
        col = hdu->tableptr + (colnum-1);
        ffmbyt(fits, hdu->datastart + row*hdu->rowlength + col->tbcol, REPORT_EOF, &status);
        if (ffgbytoff(fits, 
                      col->twidth*col->trepeat, 
                      1, 
                      0,  // spacing between elements, won't be used since reading one
                      &index[i], 
                      &status)) {
            fits_report_error(stderr, status);
            exit(EXIT_FAILURE);
        }
        colnum = 2;
        col = hdu->tableptr + (colnum-1);
        if (ffgbytoff(fits, 
                      col->twidth*col->trepeat, 
                      1, 
                      0,  // spacing between elements, won't be used since reading one
                      &x[i], 
                      &status)) {
            fits_report_error(stderr, status);
            exit(EXIT_FAILURE);
        }

        colnum = 3;
        col = hdu->tableptr + (colnum-1);
        if (ffgbytoff(fits, 
                      col->twidth*col->trepeat, 
                      1, 
                      0,  // spacing between elements, won't be used since reading one
                      &y[i], 
                      &status)) {
            fits_report_error(stderr, status);
            exit(EXIT_FAILURE);
        }


    }
    ffswap4(index, nrows2read);
    ffswap8(x, nrows2read);
    ffswap8(y, nrows2read);
    for (i=0; i<nrows2read; i++) {
        int row=rows2read[i];
        printf("row: %d  index[%d]: %d  x[%d]: %lf  y[%d]: %lf\n", row, i, index[i], i, x[i], i, y[i]);
    }



    // TODO
    //   - should we copy directly into strided array (for recarray), or go 
    //   easy and copy into unstrided array, do all scaling, byteswapping, 
    //   etc., then copy into strided?
    //   The thing is, it is more efficient to read all columns as we go,
    //   so we would want all columns in memory at once.  But the byteswapping
    //   routines and scaling routines don't accept strided arrays; would have
    //       * scaling looks really easy, as does byte swapping, can probably
    //       just roll my own
    //   to roll our own.  E.g. fffi4int
    //   - just call cfitsio routines for images

    // for column name matching, might want to just roll my own and demand
    // exact matches.  use strcmp instead of their FSTRCMP?  Case sensitive by
    // default I think.

    return 0;
}
