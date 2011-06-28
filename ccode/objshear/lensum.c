#include <stdlib.h>
#include <stdio.h>
#include "lensum.h"
#include "defs.h"

struct lensums* lensums_new(size_t nlens, size_t nbin) {
    printf("Creating lensums:\n");
    printf("    nlens: %lu  nbin: %lu\n", nlens, nbin);

    struct lensums* lensums=calloc(1,sizeof(struct lensums));
    if (lensums == NULL) {
        printf("failed to allocate lensums struct\n");
        exit(EXIT_FAILURE);
    }

    lensums->data = calloc(nlens, sizeof(struct lensum));
    if (lensums->data == NULL) {
        printf("failed to allocate lensum array\n");
        exit(EXIT_FAILURE);
    }

    lensums->size = nlens;

    struct lensum* lensum = &lensums->data[0];

    for (size_t i=0; i<nlens; i++) {
        lensum->nbin = nbin;
        lensum->npair = calloc(nbin, sizeof(int64));
        lensum->wsum  = calloc(nbin, sizeof(double));
        lensum->dsum  = calloc(nbin, sizeof(double));
        lensum->osum  = calloc(nbin, sizeof(double));
        lensum->rsum  = calloc(nbin, sizeof(double));

        if (lensum->npair==NULL
                || lensum->wsum==NULL
                || lensum->dsum==NULL
                || lensum->osum==NULL
                || lensum->rsum==NULL) {

            printf("failed to allocate lensum\n");
            exit(EXIT_FAILURE);
        }

        lensum++;
    }
    return lensums;

}

// this one we write all the data out in binary format
void lensums_write(struct lensums* lensums, FILE* fptr) {
    int64 nlens=lensums->size;
    int64 nbin=lensums->data[0].nbin;

    int res;
    res=fwrite(&nlens, sizeof(int64), 1, fptr);
    res=fwrite(&nbin, sizeof(int64), 1, fptr);

    struct lensum* lensum = &lensums->data[0];
    for (size_t i=0; i<nlens; i++) {
        res=fwrite(&lensum->zindex, sizeof(int64), 1, fptr);
        res=fwrite(&lensum->weight, sizeof(double), 1, fptr);

        res=fwrite(lensum->npair, sizeof(int64), nbin, fptr);
        res=fwrite(lensum->rsum, sizeof(double), nbin, fptr);
        res=fwrite(lensum->wsum, sizeof(double), nbin, fptr);
        res=fwrite(lensum->dsum, sizeof(double), nbin, fptr);
        res=fwrite(lensum->osum, sizeof(double), nbin, fptr);
        
        lensum++;
    }
}

struct lensum* lensums_sum(struct lensums* lensums) {
    struct lensum* tsum=lensum_new(lensums->data[0].nbin);

    struct lensum* lensum = &lensums->data[0];

    for (size_t i=0; i<lensums->size; i++) {
        tsum->weight += lensum->weight;
        tsum->totpairs += lensum->totpairs;
        for (size_t j=0; j<lensum->nbin; j++) {
            tsum->npair[j] += lensum->npair[j];
            tsum->rsum[j] += lensum->rsum[j];
            tsum->wsum[j] += lensum->wsum[j];
            tsum->dsum[j] += lensum->dsum[j];
            tsum->osum[j] += lensum->osum[j];
        }
        lensum++;
    }
    return tsum;
}



void lensums_print_sum(struct lensums* lensums) {
    struct lensum* lensum = lensums_sum(lensums);
    lensum_print(lensum);
    lensum_delete(lensum);
}

// these write the stdout
void lensums_print_one(struct lensums* lensums, size_t index) {
    printf("element %ld of lensums:\n",index);
    struct lensum* lensum = &lensums->data[index];
    lensum_print(lensum);
}

void lensums_print_firstlast(struct lensums* lensums) {
    lensums_print_one(lensums, 0);
    lensums_print_one(lensums, lensums->size-1);
}

struct lensums* lensums_delete(struct lensums* lensums) {
    if (lensums != NULL) {
        struct lensum* lensum = &lensums->data[0];

        for (size_t i=0; i<lensums->size; i++) {
            free(lensum->npair);
            free(lensum->wsum);
            free(lensum->dsum);
            free(lensum->osum);
            free(lensum->rsum);
            lensum++;
        }
    }
    free(lensums);
    return NULL;
}

struct lensum* lensum_new(size_t nbin) {
    struct lensum* lensum=calloc(1,sizeof(struct lensum));
    if (lensum == NULL) {
        printf("failed to allocate lensum\n");
        exit(EXIT_FAILURE);
    }

    lensum->nbin = nbin;

    lensum->npair = calloc(nbin, sizeof(int64));
    lensum->wsum  = calloc(nbin, sizeof(double));
    lensum->dsum  = calloc(nbin, sizeof(double));
    lensum->osum  = calloc(nbin, sizeof(double));
    lensum->rsum  = calloc(nbin, sizeof(double));

    if (lensum->npair==NULL
            || lensum->wsum==NULL
            || lensum->dsum==NULL
            || lensum->osum==NULL
            || lensum->rsum==NULL) {

        printf("failed to allocate lensum\n");
        exit(EXIT_FAILURE);
    }

    return lensum;
}

// these write the stdout
void lensum_print(struct lensum* lensum) {
    printf("  zindex:   %ld\n", lensum->zindex);
    printf("  weight:   %lf\n", lensum->weight);
    printf("  totpairs: %ld\n", lensum->totpairs);
    printf("  nbin:     %ld\n", lensum->nbin);
    printf("  bin       npair            wsum            dsum            osum           rsum\n");

    for (size_t i=0; i<lensum->nbin; i++) {
        printf("  %3lu %11ld %15.6lf %15.6lf %15.6lf   %e\n", 
               i,
               lensum->npair[i],
               lensum->wsum[i],
               lensum->dsum[i],
               lensum->osum[i],
               lensum->rsum[i]);
    }
}



void lensum_clear(struct lensum* lensum) {

    lensum->zindex=-1;
    lensum->weight=0;
    for (size_t i=0; i<lensum->nbin; i++) {
        lensum->npair[i] = 0;
        lensum->wsum[i] = 0;
        lensum->dsum[i] = 0;
        lensum->osum[i] = 0;
        lensum->rsum[i] = 0;
    }
}

struct lensum* lensum_delete(struct lensum* lensum) {
    if (lensum != NULL) {
        free(lensum->npair);
        free(lensum->wsum);
        free(lensum->dsum);
        free(lensum->osum);
        free(lensum->rsum);
    }
    free(lensum);
    return NULL;
}
