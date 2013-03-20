/*
   TODO
     - output a weight map or something?
*/
#include <stdio.h>
#include <stdlib.h>
#include <fitsio.h>
#include "image.h"
#include "image_fits.h"


/*

   We force the box size to be odd.  This is because we round the center to a
   pixel and want symmetric box.

*/
long get_box_size(const char *arg)
{
    long box_size = atol(arg);

    if ( (box_size % 2) == 0) {
        box_size += 1;
    }

    return box_size;
}


/* 
   A basic principle here is that it shouldn't matter the exact center used for
   the stacking.  So we round the row,col to the nearest pixel

   If the box goes out of bounds, we return 0, else 1
*/
int get_masks(struct image *imstack,
              const struct image *image,
              double rowcen,
              double colcen,
              struct image_mask *smask,
              struct image_mask *mask)
{
    int status=1;
    long lrow=(long)rowcen;
    long lcol=(long)colcen;

    long nrows=IM_NROWS(image);
    long ncols=IM_NCOLS(image);

    // note the stack image size is always odd
    long row_off=(IM_NROWS(imstack)-1)/2;
    long col_off=(IM_NCOLS(imstack)-1)/2;

    mask->rowmin = lrow-row_off;
    mask->rowmax = lrow+row_off;
    mask->colmin = lcol-col_off;
    mask->colmax = lcol+col_off;

    smask->rowmin = 0;
    smask->rowmax = IM_NROWS(imstack)-1;
    smask->colmin = 0;
    smask->colmax = IM_NCOLS(imstack)-1;

    if (mask->rowmin < 0
            || mask->colmin < 0
            || mask->rowmax > (nrows-1)
            || mask->colmax > (ncols-1) ) {
        status=0;
    }

    return status;
}

/*
   This version allows edge hits
*/
void get_masks_allow_edge(struct image *imstack,
                          const struct image *image,
                          double rowcen,
                          double colcen,
                          struct image_mask *smask,
                          struct image_mask *mask)
{
    long lrow=(long)rowcen;
    long lcol=(long)colcen;

    long nrows=IM_NROWS(image);
    long ncols=IM_NCOLS(image);

    // note the stack image size is always odd
    long row_off=(IM_NROWS(imstack)-1)/2;
    long col_off=(IM_NCOLS(imstack)-1)/2;

    mask->rowmin = lrow-row_off;
    mask->rowmax = lrow+row_off;
    mask->colmin = lcol-col_off;
    mask->colmax = lcol+col_off;

    smask->rowmin = 0;
    smask->rowmax = IM_NROWS(imstack)-1;
    smask->colmin = 0;
    smask->colmax = IM_NCOLS(imstack)-1;

    if (mask->rowmin < 0) {
        long del=-mask->rowmin;
        mask->rowmin  += del;
        smask->rowmin += del;
    }
    if (mask->colmin < 0) {
        long del=-mask->colmin;
        mask->colmin  += del;
        smask->colmin += del;
    }


    if (mask->rowmax > (nrows-1)) {
        long del=mask->rowmax-(nrows-1);
        mask->rowmax  -= del;
        smask->rowmax -= del;
    }
    if (mask->colmax > (ncols-1)) {
        long del=mask->colmax-(ncols-1);
        mask->colmax  -= del;
        smask->colmax -= del;
    }
}


/*
   Add pixels from the main image to the stack

   The masks should already fit within the bounds of each image
*/
void add_to_stack(struct image *imstack,
                  const struct image *image,
                  struct image_mask *smask,
                  struct image_mask *mask)
{

    for (long srow=smask->rowmin, irow=mask->rowmin; 
             irow<=mask->rowmax; srow++, irow++) {

        double *stack_rowdata=IM_ROW(imstack, srow);
        double *rowdata=IM_ROW(image, irow);

        for (long scol=smask->colmin, icol=mask->colmin; 
                icol<=mask->colmax; scol++, icol++) {

            stack_rowdata[scol] += rowdata[icol];
        }
    }

}

/*
   Wow this is a lot of code to get a header key
*/
double get_sky(const char *filename, int ext)
{
    fitsfile* fits=NULL;
    char comment[FLEN_COMMENT];

    int status=0;
    if (fits_open_file(&fits, filename, READONLY, &status)) {
        fits_report_error(stderr, status);
        exit(EXIT_FAILURE);
    }

    int hdutype=0;
    if (fits_movabs_hdu(fits, ext+1, &hdutype, &status)) {
        fits_report_error(stderr, status);
        exit(EXIT_FAILURE);
    }

    double sky=0;
    if (fits_read_key_dbl(fits,"sky",&sky,comment,&status)) {
        fits_report_error(stderr, status);
        exit(EXIT_FAILURE);
    }

    if (fits_close_file(fits, &status)) {
        fits_report_error(stderr, status);
        exit(EXIT_FAILURE);
    }

    return sky;

}

int main(int argc, char **argv)
{
    if (argc < 4) {
        fprintf(stderr,"imstack image_in image_out box_size < catalog\n");
        fprintf(stderr,"  catalog should have two columns, row col\n");
        fprintf(stderr,"  box_size will be forced odd\n");
        exit(EXIT_FAILURE);
    }
    const char *image_file_in=argv[1];
    const char *image_file_out=argv[2];
    long box_size = get_box_size(argv[3]);

    printf("reading %s\n", image_file_in);
    struct image *image=image_read_fits(image_file_in,0);
    double sky=get_sky(image_file_in,0);
    image_add_scalar(image, (-sky));

    struct image *imstack=image_new(box_size, box_size);

    struct image_mask mask={0};
    struct image_mask smask={0};
    double row=0, col=0;

    long ntot=0, nedge=0, nuse=0;
    while (2==fscanf(stdin,"%lf %lf", &row, &col)) {
        int status=get_masks(imstack, image, row, col, &smask, &mask);
        if (status) {
            add_to_stack(imstack, image, &smask, &mask);
            nuse++;
        } else {
            nedge++;
        }

        ntot++;
    }

    printf("nuse: %ld/%ld  nedge: %ld/%ld\n",
            nuse,ntot,nedge,ntot);
    printf("writing to %s\n", image_file_out);
    int clobber=1;
    image_write_fits(imstack, image_file_out, clobber);
}
