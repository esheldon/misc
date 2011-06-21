#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "math.h"
#include "lens.h"

struct lcat* lcat_new(size_t n_lens) {

    if (n_lens == 0) {
        printf("lcat_new: size must be > 0\n");
        exit(EXIT_FAILURE);
    }

    struct lcat* lcat = malloc(sizeof(struct lcat));
    if (lcat == NULL) {
        printf("Could not allocate struct lcat\n");
        exit(EXIT_FAILURE);
    }

    lcat->size = n_lens;

    lcat->data = malloc(n_lens*sizeof(struct lens));
    if (lcat->data == NULL) {
        printf("Could not allocate %ld lenses in lcat\n", n_lens);
        exit(EXIT_FAILURE);
    }

    return lcat;
}

struct lcat* lcat_read(const char* filename) {
    printf("Reading lenses from %s\n", filename);
    FILE* fptr=fopen(filename,"r");
    if (fptr==NULL) {
        printf("Could not open file: %s\n", filename);
        exit(EXIT_FAILURE);
    }

    int rval;
    int64 nlens;

    rval=fread(&nlens, sizeof(int64), 1, fptr);
    printf("Reading %ld lenses\n", nlens);
    printf("  creating lcat...");

    struct lcat* lcat=lcat_new(nlens);

    struct lens* lens = &lcat->data[0];
    double ra_rad,dec_rad;
    for (size_t i=0; i<nlens; i++) {
        rval=fread(&lens->ra, sizeof(double), 1, fptr);
        rval=fread(&lens->dec, sizeof(double), 1, fptr);
        rval=fread(&lens->z, sizeof(double), 1, fptr);
        // remove these from file since we calculate!
        rval=fread(&lens->dc, sizeof(double), 1, fptr);
        rval=fread(&lens->zindex, sizeof(int64), 1, fptr);

        ra_rad = lens->ra*D2R;
        dec_rad = lens->dec*D2R;

        lens->sinra = sin(ra_rad);
        lens->cosra = cos(ra_rad);
        lens->sindec = sin(dec_rad);
        lens->cosdec = cos(dec_rad);

    }
}

// use like this:
//   lcat = lcat_delete(lcat);
// This ensures that the lcat pointer is set to NULL
struct lcat* lcat_delete(struct lcat* lcat) {

    if (lcat != NULL) {
        free(lcat->data);
        free(lcat);
    }
    return NULL;
}
