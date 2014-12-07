#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

/* 
 * The data are the sums 
 *    sum(weight*data)
 * and
 *    sum(weight)
 * The data is laid out as
 *    sum(weight*data)_1, ... sum(weight*data)_nvar  sum(weight)_1, ... sum(weight)_nvar
 *
 *  for lensing the var indices correspond to radii
 *
 * 
 *  The output is first the mean,err for each var in columns.  This is followed
 *  by the covariance matrix in rows
 */

struct data {
    int64_t nvar;
    int64_t nsample;
    double* varsums;
    double* wsums;

    double* mean;
    double* covar;
};

struct data* data_new(int64_t nsample, int64_t nvar) {

    struct data* data;

    data = calloc(1, sizeof(struct data));

    if (NULL==data) {
        printf("could not allocate data\n");
        exit(EXIT_FAILURE);
    }

    data->nvar=nvar;
    data->nsample=nsample;

    data->varsums = calloc(nsample*nvar, sizeof(double));
    if (NULL==data->varsums) {
        printf("could not allocate data->varsums\n");
        exit(EXIT_FAILURE);
    }


    data->wsums = calloc(nsample*nvar, sizeof(double));
    if (NULL==data->wsums) {
        printf("could not allocate data->wsums\n");
        exit(EXIT_FAILURE);
    }

    data->mean = calloc(nvar, sizeof(double));
    if (NULL==data->mean) {
        printf("could not allocate data->mean\n");
        exit(EXIT_FAILURE);
    }

    data->covar = calloc(nvar*nvar, sizeof(double));
    if (NULL==data->covar) {
        printf("could not allocate data->covar\n");
        exit(EXIT_FAILURE);
    }

    return data;
}


/* count the tokens in the string */
int get_ntok(char* string) {
    char* delim=" \t\n";

    int ntok=0;

    char* tmp = strtok(string, delim);
    if (tmp == NULL) {
        printf("error: no tokens found\n");
        exit(EXIT_FAILURE);
    }
    ntok++;
    while ((tmp=strtok(NULL,delim)) != NULL) {
        ntok++;
    }

    return ntok;
}

void get_data_info(FILE* fptr, int* nvar, int* nsample) {
    size_t nbytes = 100;
    int ntok;

    /* this size is just a starting point */
    char* line = (char *) malloc (nbytes + 1);
    int bytes_read = getline(&line, &nbytes, fptr);

    if (bytes_read == -1) {
        printf("  error reading first line\n");
        exit(EXIT_FAILURE);
    }

    ntok = get_ntok(line);
    if ((ntok % 2) != 0) {
        printf("error: number of entries in the line must be a multiple of two\n");
        exit(EXIT_FAILURE);
    }
    *nvar = ntok/2;
    *nsample=1;

    /* now count lines */
    while ((bytes_read=getline(&line, &nbytes, fptr)) != -1) {
        *nsample += 1;
    }

    free(line);
    return;

}

FILE* open_file(const char* filename, const char* mode) {
    FILE* fptr = fopen(filename, mode);
    if (fptr==NULL) {
        printf("Could not open file %s with mode %s\n", filename, mode);
        exit(EXIT_FAILURE);
    }
    return fptr;
}

struct data* data_read(const char* filename) {

    int64_t nsample, nvar;
    int ret;

    FILE* fptr = open_file(filename,"r");

    ret=fread(&nsample, sizeof(int64_t), 1, fptr);
    if (ret != 1) {
        printf("could not read nsample\n");
        exit(EXIT_FAILURE);
    }
    ret=fread(&nvar, sizeof(int64_t), 1, fptr);
    if (ret != 1) {
        printf("could not read nvar\n");
        exit(EXIT_FAILURE);
    }

    printf("nsample: %ld\n", nsample);
    printf("nvar: %ld\n", nvar);

    struct data* data = data_new(nsample, nvar);

    int64_t nread = nsample*nvar;
    ret=fread(data->varsums, sizeof(int64_t), nread, fptr);
    if (ret != nread) {
        printf("could not read %ld varsums\n", nread);
        exit(EXIT_FAILURE);
    }
    ret=fread(data->wsums, sizeof(int64_t), nread, fptr);
    if (ret != nread) {
        printf("could not read %ld wsums\n", nread);
        exit(EXIT_FAILURE);
    }

    fclose(fptr);
    return data;
}



void data_print_one(struct data* data, int64_t index) {

    double* varsums = &data->varsums[index*data->nvar];
    double* wsums   = &data->wsums[index*data->nvar];

    printf("index %ld\n", index);
    printf("  varsums:  ");
    for (int64_t i=0; i<data->nvar; i++) {
        printf("%+e ", varsums[i]);
    }
    printf("\n");
    printf("  wsums:    ");
    for (int64_t i=0; i<data->nvar; i++) {
        printf("%+e ", wsums[i]);
    }
    printf("\n");

}

void jackknife(struct data* data) {

    double* varsums = data->varsums;
    double* wsums   = data->wsums;

    int nvar=data->nvar;
    int nsample=data->nsample;
    double* jdiff = calloc(data->nvar, sizeof(double));

    // this gets summed across all samples
    double* vsum_tot = calloc(data->nvar, sizeof(double));
    double* wsum_tot = calloc(data->nvar, sizeof(double));

    // first calculate the overall sums
    for (int64_t i=0; i<nsample; i++) {
        for (int64_t j=0; j<nvar; j++) {
            vsum_tot[j] += *varsums;
            wsum_tot[j] += *wsums;
            varsums++;
            wsums++;
        }
    }

    // now the overall mean
    for (int64_t j=0; j<nvar; j++) {
        if (wsum_tot[j] > 0) {
            data->mean[j] = vsum_tot[j]/wsum_tot[j];
        }
    }

    // now jackknife the covariance
    double jmean=0, val=0;
    varsums = data->varsums;
    wsums   = data->wsums;
    double* covar = data->covar;
    for (int64_t i=0; i<nsample; i++) {
        // mean with this sample subtracted
        for (int64_t j=0; j<nvar; j++) {
            double twsum = (wsum_tot[j]-*wsums);
            if (twsum > 0) {
                jmean = (vsum_tot[j]-*varsums)/twsum;
                jdiff[j] = jmean-data->mean[j];
            } else {
                jdiff[j] = 9999;
            }
            varsums++;
            wsums++;
        }

        // now grab all the cross terms
        for (int64_t ix=0; ix<nvar; ix++) {
            for (int64_t iy=ix; iy<nvar; iy++) {
                val = jdiff[ix]*jdiff[iy];

                covar[ix*nvar + iy] += val;
                if (ix != iy) {
                    covar[iy*nvar + ix] += val;
                }
            }
        }
    }

    // note jackknife normalization copared to normal variance
    double fnsample = (double)nsample;
    double norm = (fnsample-1.)/nsample;
    for (int64_t ix=0; ix<nvar; ix++) {
        for (int64_t iy=0; iy<nvar; iy++) {
            covar[ix*nvar + iy] *= norm;
        }
    }


    free(jdiff);
    free(vsum_tot);
    free(wsum_tot);
}

void data_print(struct data* data, const char* filename) {
    FILE* fptr = open_file(filename,"w");

    int64_t nvar=data->nvar;
    fprintf(fptr, "%ld\n", nvar);

    // first print mean, err in columns
    double* covar=data->covar;
    for (int64_t ix=0; ix<nvar; ix++) {
        fprintf(fptr, "%+.15e %.15e\n", data->mean[ix], sqrt(covar[ix*nvar + ix]));
    }

    // now the full covariance matrix in rows
    for (int64_t ix=0; ix<nvar; ix++) {
        for (int64_t iy=0; iy<nvar; iy++) {

            fprintf(fptr, "%+.15e", covar[ix*nvar + iy]);
            if (iy == (nvar-1)) {
                fprintf(fptr, "\n");
            } else {
                fprintf(fptr, " ");
            }
        }
    }

    fclose(fptr);
}

int main(int argc, char** argv) {

    const char* input_filename;
    const char* output_filename;
    struct data* data=NULL;

    if (argc < 3) {
        printf("jackknife input_filename output_filename\n");
        exit(EXIT_FAILURE);
    }

    input_filename = argv[1];
    output_filename = argv[2];

    data = data_read(input_filename);

    data_print_one(data, 0);
    data_print_one(data, data->nsample-1);

    jackknife(data);

    data_print(data, output_filename);
}
