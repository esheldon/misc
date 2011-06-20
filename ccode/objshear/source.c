#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "math.h"
#include "source.h"

#ifdef SOURCE_POFZ
#include "Vector.h"
struct scat* scat_new(size_t n_source, size_t n_zlens) {
    if (n_zlens == 0) {
        printf("scat_new: n_zlens must be > 0\n");
        exit(EXIT_FAILURE);
    }
#else
struct scat* scat_new(size_t n_source) {
#endif

    if (n_source == 0) {
        printf("scat_new: size must be > 0\n");
        exit(EXIT_FAILURE);
    }

    struct scat* scat = malloc(sizeof(struct scat));
    if (scat == NULL) {
        printf("Could not allocate struct scat\n");
        exit(EXIT_FAILURE);
    }

    scat->size = n_source;

    scat->data = malloc(n_source*sizeof(struct source));
    if (scat->data == NULL) {
        printf("Could not allocate %ld sources in scat\n", n_source);
        exit(EXIT_FAILURE);
    }

#ifdef SOURCE_POFZ

    scat->zlens = f64vector_new(n_zlens);
    if (scat->zlens== NULL) {
        printf("Could not allocate %ld zlens in scat\n", n_zlens);
        exit(EXIT_FAILURE);
    }

    struct source* src = scat->data[0];
    for (size_t i=0; i<scat->size; i++) {
        src->scinv = f64vector_new(n_zlens);

        if (scat->scinv == NULL) {
            printf("Could not allocate %ld scinv in scat[%ld]\n", n_zlens, i);
            exit(EXIT_FAILURE);
        }

        // for convenience point to zlens in each source structure
        // DONT FREE!
        src->zlens = zlens;
        src++;
    }
#endif

    return scat;
}

struct scat* scat_read(const char* filename) {
    printf("Reading sources from %s\n", filename);
    FILE* fptr=fopen(filename,"r");
    if (fptr==NULL) {
        printf("Could not open file: %s\n", filename);
        exit(EXIT_FAILURE);
    }

    int64 sigmacrit_style;
    fread(&sigmacrit_style, sizeof(int64), 1, fptr);

#ifdef SOURCE_POFZ
    if (sigmacrit_style != 2) {
        printf("Got sigmacrit_style = %ld but code is compiled for p(z), sigmacrit_style=2.\n", sigmacrit_style);
        exit(EXIT_FAILURE);
    }
#else
    if (sigmacrit_style != 1) {
        printf("Got sigmacrit_style = %ld but code is compiled for true z, sigmacrit_style=1.\n", sigmacrit_style);
        exit(EXIT_FAILURE);
    }

    int64 nsource;
    fread(&nsource, sizeof(int64), 1, fptr);
    printf("Reading %ld sources\n", nsource);
    printf("  creating scat...");
    struct scat* scat=scat_new(nsource);
    printf("OK\n");

    printf("  reading data...");
    double ra,dec;
    struct source* src=scat->data;

    for (size_t i=0; i<scat->size; i++) {
    //for (size_t i=0; i<1; i++) {
        fread(&ra, sizeof(double), 1, fptr);
        fread(&dec, sizeof(double), 1, fptr);
        fread(&src->g1, sizeof(double), 1, fptr);
        fread(&src->g2, sizeof(double), 1, fptr);
        fread(&src->err, sizeof(double), 1, fptr);

        // need to remove this from the file since we
        // always calculate it
        fread(&src->hpixid, sizeof(int64), 1, fptr);

        fread(&src->z, sizeof(double), 1, fptr);

        // don't forget to calculate dc!
        fread(&src->dc, sizeof(double), 1, fptr);

        if (i == 0 || i == (scat->size-1)) {
            printf("  %ld: ra: %lf  dec: %lf", i, ra, dec);
        }

        ra *= D2R;
        dec *= D2R;
        src->sinra = sin(ra);
        src->cosra = cos(ra);
        src->sindec = sin(dec);
        src->cosdec = cos(dec);

        src++;
    }
    printf("OK\n");

#endif

    return scat;
}

void scat_print_one(struct scat* scat, size_t el) {
    struct source* src = &scat->data[el];
    printf("element %ld of scat:\n", el);
    printf("  ra: %lf  dec: %lf\n", R2D*asin(src->sinra), R2D*asin(src->sindec));
    printf("  g1: %lf  g2: %lf\n", src->g1, src->g2);
    printf("  err: %lf\n", src->err);
    printf("  hpixid: %ld\n", src->hpixid);
}
void scat_print_firstlast(struct scat* scat) {
    scat_print_one(scat, 0);
    scat_print_one(scat, scat->size-1);
}

// use like this:
//   scat = scat_delete(scat);
// This ensures that the scat pointer is set to NULL
struct scat* scat_delete(struct scat* scat) {

    if (scat != NULL) {

#ifdef SOURCE_POFZ
        f64vector_delete(scat->zlens);

        struct source* src = scat->data[0];
        for (size_t i=0; i<scat->size; i++) {
            free(src->scinv);
            src++;
        }
#endif

        free(scat->data);
        free(scat);
    }
    return NULL;
}
