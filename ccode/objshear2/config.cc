#include <stdio.h>
#include <stdlib.h>
#include <cmath>
#include "config.h"

void read_config(const char* filename, struct config& pars) {
    FILE* fptr;
    printf("Reading config file %s\n", filename);
    if (! (fptr = fopen(filename, "r")) ) {
        printf("Cannot open config file %s\n", filename);
        exit(45);
    }

    char keyword[255];
    int nread;

    nread = fscanf(fptr, "%s %s", keyword, pars.lens_file);
    nread = fscanf(fptr, "%s %s", keyword, pars.source_file);
    nread = fscanf(fptr, "%s %s", keyword, pars.rev_file);

    nread = fscanf(fptr, "%s %s", keyword, pars.output_file);

    nread = fscanf(fptr, "%s %f", keyword, &pars.H0);
    nread = fscanf(fptr, "%s %f", keyword, &pars.omega_m);

    nread = fscanf(fptr, "%s %d", keyword, &pars.sigmacrit_style);

    nread = fscanf(fptr, "%s %d", keyword, &pars.nbin);

    nread = fscanf(fptr, "%s %f", keyword, &pars.rmin);
    nread = fscanf(fptr, "%s %f", keyword, &pars.rmax);


    pars.log_rmin = log10(pars.rmin);
    pars.log_rmax = log10(pars.rmax);
    pars.log_binsize = ( pars.log_rmax - pars.log_rmin )/pars.nbin;

    fclose(fptr);

}

void print_config(struct config& pars) {

    printf("lens_file         %s\n", pars.lens_file);
    printf("source_file       %s\n", pars.source_file);
    printf("rev_file          %s\n", pars.rev_file);
    printf("output_file       %s\n", pars.output_file);
    printf("H0                %f\n", pars.H0);
    printf("omega_m           %f\n", pars.omega_m);
    printf("sigmacrit_style   %d\n", pars.sigmacrit_style);
    printf("nbin              %d\n", pars.nbin);
    printf("rmin              %f\n", pars.rmin);
    printf("rmax              %f\n", pars.rmax);
    printf("log_rmin          %f\n", pars.log_rmin);
    printf("log_rmax          %f\n", pars.log_rmax);
    printf("log_binsize       %f\n", pars.log_binsize);

}
