#ifndef _CONFIG_HEADER
#define _CONFIG_HEADER

#include "defs.h"

struct config {
    char lens_file[255];
    char source_file[255];
    char output_file[255];

    double H0;
    double omega_m;
    int64 npts;  // for cosmo integration

    int64 nside; // hpix

    int64 sigmacrit_style;

    int64 nbin;
    double rmin;
    double rmax;

    // we fill these in
    double log_rmin;
    double log_rmax;
    double log_binsize;

};

struct config* read_config(const char* filename);
void print_config(struct config* config);

#endif
