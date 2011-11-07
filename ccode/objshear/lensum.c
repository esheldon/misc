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

void lensums_write_header(size_t nlens, size_t nbin, FILE* fptr) {
    fprintf(fptr, "SIZE = %ld\n", nlens);
    fprintf(fptr, "{'_DTYPE': [('zindex',   'i8'),\n");
    fprintf(fptr, "            ('weight',   'f8'),\n");
    fprintf(fptr, "            ('totpairs', 'i8'),\n");
    fprintf(fptr, "            ('sshsum',   'f8'),\n");
    fprintf(fptr, "            ('npair',    'i8', %ld),\n", nbin);
    fprintf(fptr, "            ('rsum',     'f8', %ld),\n", nbin);
    fprintf(fptr, "            ('wsum',     'f8', %ld),\n", nbin);
    fprintf(fptr, "            ('dsum',     'f8', %ld),\n", nbin);
    fprintf(fptr, "            ('osum',     'f8', %ld)],\n", nbin);
    fprintf(fptr, " '_VERSION': '1.0'}\n");
    fprintf(fptr, "END\n");
    fprintf(fptr, "\n");
}

// this is for writing them all at once.  We actually usually
// write them one at a time
void lensums_write(struct lensums* lensums, FILE* fptr) {
    int64 nlens=lensums->size;
    int64 nbin=lensums->data[0].nbin;

    lensums_write_header(nlens, nbin, fptr);

    struct lensum* lensum = &lensums->data[0];
    for (size_t i=0; i<nlens; i++) {
        lensum_write(lensum, fptr);
        
        lensum++;
    }
}

// this one we write all the data out in binary format
void lensums_write_old(struct lensums* lensums, FILE* fptr) {
    int64 nlens=lensums->size;
    int64 nbin=lensums->data[0].nbin;

    fwrite(&nlens, sizeof(int64), 1, fptr);
    fwrite(&nbin, sizeof(int64), 1, fptr);

    struct lensum* lensum = &lensums->data[0];
    for (size_t i=0; i<nlens; i++) {
        fwrite(&lensum->zindex, sizeof(int64), 1, fptr);
        fwrite(&lensum->weight, sizeof(double), 1, fptr);

        fwrite(lensum->npair, sizeof(int64), nbin, fptr);
        fwrite(lensum->rsum, sizeof(double), nbin, fptr);
        fwrite(lensum->wsum, sizeof(double), nbin, fptr);
        fwrite(lensum->dsum, sizeof(double), nbin, fptr);
        fwrite(lensum->osum, sizeof(double), nbin, fptr);
        
        lensum++;
    }
}

struct lensum* lensums_sum(struct lensums* lensums) {
    struct lensum* tsum=lensum_new(lensums->data[0].nbin);

    struct lensum* lensum = &lensums->data[0];

    for (size_t i=0; i<lensums->size; i++) {
        tsum->weight   += lensum->weight;
        tsum->totpairs += lensum->totpairs;
        tsum->sshsum   += lensum->sshsum;
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


// add the second lensum into the first
void lensum_add(struct lensum* dest, struct lensum* src) {

    dest->weight   += src->weight;
    dest->totpairs += src->totpairs;
    dest->sshsum   += src->sshsum;
    for (size_t i=0; i<src->nbin; i++) {
        dest->npair[i] += src->npair[i];
        dest->rsum[i] += src->rsum[i];
        dest->wsum[i] += src->wsum[i];
        dest->dsum[i] += src->dsum[i];
        dest->osum[i] += src->osum[i];
    }

}

void lensum_write(struct lensum* lensum, FILE* fptr) {
    int nbin = lensum->nbin;

    fwrite(&lensum->zindex, sizeof(int64), 1, fptr);
    fwrite(&lensum->weight, sizeof(double), 1, fptr);
    fwrite(&lensum->totpairs, sizeof(int64), 1, fptr);

    fwrite(&lensum->sshsum, sizeof(double), 1, fptr);

    fwrite(lensum->npair, sizeof(int64), nbin, fptr);
    fwrite(lensum->rsum, sizeof(double), nbin, fptr);
    fwrite(lensum->wsum, sizeof(double), nbin, fptr);
    fwrite(lensum->dsum, sizeof(double), nbin, fptr);
    fwrite(lensum->osum, sizeof(double), nbin, fptr);
}

// these write the stdout
void lensum_print(struct lensum* lensum) {
    printf("  zindex:   %ld\n", lensum->zindex);
    printf("  weight:   %lf\n", lensum->weight);
    printf("  sshsum:   %lf\n", lensum->sshsum);
    printf("  ssh:      %lf\n", lensum->sshsum/lensum->weight);
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
    lensum->totpairs=0;
    lensum->sshsum=0;
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
