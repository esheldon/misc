#include <stdlib.h>
#include <stdio.h>
#include "lensum.h"
#include "defs.h"
#include "log.h"

struct lensums* lensums_new(size_t nlens, size_t nbin) {
    wlog("Creating lensums:\n");
    wlog("    nlens: %lu  nbin: %lu\n", nlens, nbin);

    struct lensums* lensums=calloc(1,sizeof(struct lensums));
    if (lensums == NULL) {
        wlog("failed to allocate lensums struct\n");
        exit(EXIT_FAILURE);
    }

    lensums->data = calloc(nlens, sizeof(struct lensum));
    if (lensums->data == NULL) {
        wlog("failed to allocate lensum array\n");
        exit(EXIT_FAILURE);
    }

    lensums->size = nlens;

    struct lensum* lensum = &lensums->data[0];

    for (size_t i=0; i<nlens; i++) {
        lensum->index = i;
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

            wlog("failed to allocate lensum\n");
            exit(EXIT_FAILURE);
        }

        lensum++;
    }
    return lensums;

}

// this one we write all the data out in .rec binary format
/* 
SIZE =               206951
{'_DTYPE': [('zindex', '<i8'),
            ('weight', '<f8'),
            ('npair', '<i8', 21),
            ('rsum', '<f8', 21),
            ('wsum', '<f8', 21),
            ('dsum', '<f8', 21),
            ('osum', '<f8', 21)],
 '_VERSION': '1.0'}
END
*/


// this is for writing them all at once.  We actually usually
// write them one at a time
void lensums_write(struct lensums* lensums, FILE* stream) {

    struct lensum* lensum = &lensums->data[0];
    for (size_t i=0; i<lensums->size; i++) {
        lensum_write(lensum, stream);
        
        lensum++;
    }
}


struct lensum* lensums_sum(struct lensums* lensums) {
    struct lensum* tsum=lensum_new(lensums->data[0].nbin);

    struct lensum* lensum = &lensums->data[0];

    for (size_t i=0; i<lensums->size; i++) {
        tsum->weight   += lensum->weight;
        tsum->totpairs += lensum->totpairs;
        tsum->x2sum += lensum->x2sum;
        tsum->y2sum += lensum->y2sum;
        tsum->xysum += lensum->xysum;
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
    lensum=lensum_delete(lensum);
}

// these write the stdout
void lensums_print_one(struct lensums* lensums, size_t index) {
    wlog("element %ld of lensums:\n",index);
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
        wlog("failed to allocate lensum\n");
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

        wlog("failed to allocate lensum\n");
        exit(EXIT_FAILURE);
    }

    return lensum;
}

// add the second lensum into the first
void lensum_add(struct lensum* dest, struct lensum* src) {

    dest->weight   += src->weight;
    dest->totpairs += src->totpairs;
    dest->x2sum += src->x2sum;
    dest->y2sum += src->y2sum;
    dest->xysum += src->xysum;
    for (size_t i=0; i<src->nbin; i++) {
        dest->npair[i] += src->npair[i];
        dest->rsum[i] += src->rsum[i];
        dest->wsum[i] += src->wsum[i];
        dest->dsum[i] += src->dsum[i];
        dest->osum[i] += src->osum[i];
    }

}

int lensum_read(FILE* stream, struct lensum* lensum) {
    int nbin=lensum->nbin;
    int nexpect = 7+5*nbin;
    int nread=0;
    int i=0;

    nread+=fscanf(stream,"%ld", &lensum->index);
    nread+=fscanf(stream,"%ld", &lensum->zindex);
    nread+=fscanf(stream,"%lf", &lensum->weight);
    nread+=fscanf(stream,"%ld", &lensum->totpairs);

    nread+=fscanf(stream,"%lf", &lensum->x2sum);
    nread+=fscanf(stream,"%lf", &lensum->y2sum);
    nread+=fscanf(stream,"%lf", &lensum->xysum);

    for (i=0; i<nbin; i++) 
        nread+=fscanf(stream,"%ld", &lensum->npair[i]);

    for (i=0; i<nbin; i++) 
        nread+=fscanf(stream,"%lf", &lensum->rsum[i]);
    for (i=0; i<nbin; i++) 
        nread+=fscanf(stream,"%lf", &lensum->wsum[i]);
    for (i=0; i<nbin; i++) 
        nread+=fscanf(stream,"%lf", &lensum->dsum[i]);
    for (i=0; i<nbin; i++) 
        nread+=fscanf(stream,"%lf", &lensum->osum[i]);

    return (nread == nexpect);
}

void lensum_write(struct lensum* lensum, FILE* stream) {
    int nbin = lensum->nbin;
    int i=0;

    // id with tab at beginning is demanded by hadoop map reduce
    // should we just use zindex there?
    fprintf(stream,"%ld\t%ld %.16g %ld %.16g %.16g %.16g ", 
            lensum->index, lensum->zindex, lensum->weight, lensum->totpairs,
            lensum->x2sum, lensum->y2sum, lensum->xysum);

    for (i=0; i<nbin; i++) 
        fprintf(stream,"%ld ", lensum->npair[i]);

    for (i=0; i<nbin; i++) 
        fprintf(stream,"%.16g ", lensum->rsum[i]);
    for (i=0; i<nbin; i++) 
        fprintf(stream,"%.16g ", lensum->wsum[i]);
    for (i=0; i<nbin; i++) 
        fprintf(stream,"%.16g ", lensum->dsum[i]);
    for (i=0; i<nbin-1; i++) 
        fprintf(stream,"%.16g ", lensum->osum[i]);
    fprintf(stream,"%.16g\n", lensum->osum[nbin-1]);

}

// these write the stdout
void lensum_print(struct lensum* lensum) {
    wlog("  zindex:   %ld\n", lensum->zindex);
    wlog("  weight:   %lf\n", lensum->weight);
    wlog("  totpairs: %ld\n", lensum->totpairs);
    wlog("  x2sum:    %lf\n", lensum->x2sum);
    wlog("  y2sum:    %lf\n", lensum->y2sum);
    wlog("  xysum:    %lf\n", lensum->xysum);
    wlog("  nbin:     %ld\n", lensum->nbin);
    wlog("  bin       npair            wsum            dsum            osum           rsum meanr\n");

    for (size_t i=0; i<lensum->nbin; i++) {
        wlog("  %3lu %11ld %15.6lf %15.6lf %15.6lf   %e %g\n", 
             i,
             lensum->npair[i],
             lensum->wsum[i],
             lensum->dsum[i],
             lensum->osum[i],
             lensum->rsum[i],
             lensum->rsum[i]/lensum->npair[i]);
    }
}

struct lensum* lensum_copy(struct lensum* lensum) {
    int i=0;

    struct lensum* copy=lensum_new(lensum->nbin);
    copy->index = lensum->index;
    copy->zindex = lensum->zindex;
    copy->weight = lensum->weight;
    copy->totpairs = lensum->totpairs;

    copy->x2sum = lensum->x2sum;
    copy->y2sum = lensum->y2sum;
    copy->xysum = lensum->xysum;

    copy->nbin = lensum->nbin;

    for (i=0; i<copy->nbin; i++) {
        copy->npair[i] = lensum->npair[i];
        copy->rsum[i] = lensum->rsum[i];
        copy->wsum[i] = lensum->wsum[i];
        copy->dsum[i] = lensum->dsum[i];
        copy->osum[i] = lensum->osum[i];
    }

    return copy;
}




void lensum_clear(struct lensum* lensum) {

    lensum->zindex=-1;
    lensum->weight=0;
    lensum->totpairs=0;

    lensum->x2sum = 0.;
    lensum->y2sum = 0.;
    lensum->xysum = 0.;

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
        free(lensum->rsum);
        free(lensum->wsum);
        free(lensum->dsum);
        free(lensum->osum);
    }
    free(lensum);
    return NULL;
}
