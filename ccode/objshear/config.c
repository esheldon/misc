#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "config.h"

struct config* config_read(const char* filename) {
    int nread=0, expect=22;
    printf("Reading config from %s\n", filename);
    FILE* fptr=fopen(filename,"r");
    if (fptr==NULL) {
        printf("Could not open file: %s\n", filename);
        exit(EXIT_FAILURE);
    }

    struct config* c=calloc(1, sizeof(struct config));

    char key[255];
    nread += fscanf(fptr, "%s %s", key, c->lens_file);
    nread += fscanf(fptr, "%s %s", key, c->source_file);
    nread += fscanf(fptr, "%s %s", key, c->output_file);
    nread += fscanf(fptr, "%s %lf", key, &c->H0);
    nread += fscanf(fptr, "%s %lf", key, &c->omega_m);
    nread += fscanf(fptr, "%s %ld", key, &c->npts);
    nread += fscanf(fptr, "%s %ld", key, &c->nside);
    nread += fscanf(fptr, "%s %ld", key, &c->sigmacrit_style);
    nread += fscanf(fptr, "%s %ld", key, &c->nbin);
    nread += fscanf(fptr, "%s %lf", key, &c->rmin);
    nread += fscanf(fptr, "%s %lf", key, &c->rmax);

    if (nread != expect) {
        fprintf(stderr,"expected to read %d got %d\n", expect, nread);
        exit(EXIT_FAILURE);
    }
    c->log_rmin = log10(c->rmin);
    c->log_rmax = log10(c->rmax);
    c->log_binsize = (c->log_rmax - c->log_rmin)/c->nbin;

    fclose(fptr);

    return c;
}

// usage:  config=config_delete(config);
struct config* config_delete(struct config* config) {
    free(config);
    return NULL;
}

void config_print(struct config* c) {
    printf("    lens_file:    %s\n", c->lens_file);
    printf("    source_file:  %s\n", c->source_file);
    printf("    output_file:  %s\n", c->output_file);
    //printf("    output_file:  %s\n", c->temp_file);
    printf("    H0:           %lf\n", c->H0);
    printf("    omega_m:      %lf\n", c->omega_m);
    printf("    npts:         %ld\n", c->npts);
    printf("    nside:        %ld\n", c->nside);
    printf("    scrit style:  %ld\n", c->sigmacrit_style);
    printf("    nbin:         %ld\n", c->nbin);
    printf("    rmin:         %lf\n", c->rmin);
    printf("    rmax:         %lf\n", c->rmax);
    printf("    log(rmin):    %lf\n", c->log_rmin);
    printf("    log(rmax):    %lf\n", c->log_rmax);
    printf("    log(binsize): %lf\n", c->log_binsize);
}
