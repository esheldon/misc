#include <string.h>
#include <stdio.h>
#include <stdlib.h>


#include "fitsio.h"


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
    const char* filename="test.fits";

    fitsfile* fits=NULL;
    FITSfile* hdu=NULL;
    tcolumn* col=NULL;

    int status=0;
    int ncols=0;
    long nrows=0;

    int nfound=0, i=0;
    int hdunum = 2, hdutype=0;

    printf("opening: %s\n", filename);
    if (fits_open_file(&fits, filename, READONLY, &status) != 0) {
        fits_report_error(stderr, status);
        exit(EXIT_FAILURE);
    }

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

    // for column name matching, might want to just roll my own and demand
    // exact matches.  use strcmp instead of their FSTRCMP?  Case sensitive by
    // default I think.

    // TODO
    //   - getting certain rows
    //   - should we copy into strided array (for recarray), or go easy and copy into
    //     unstrided array, then copy into strided?  does that actually save us any
    //     time considering a copy must be made?

    return 0;
}
