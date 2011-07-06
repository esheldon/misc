#ifndef _SHEAR_HEADER
#define _SHEAR_HEADER

#include <stdio.h>
#include "defs.h"
#include "config.h"
#include "cosmo.h"
#include "healpix.h"
#include "lens.h"
#include "lensum.h"
#include "source.h"

struct shear {
    struct config*  config;
    struct cosmo*   cosmo;
    struct healpix* hpix;
    struct scat*    scat;
    struct lcat*    lcat;

    // hold pixels
    struct i64stack* pixstack;

    // this holds the info for a given lens
    struct lensums* lensums;

    // output file pointer. We open at the beginning to make sure we can!
    FILE* fptr;
};

struct shear* shear_init(const char* config_filename);
struct shear* shear_delete(struct shear* shear);

void shear_calc(struct shear* shear);

void shear_print_sum(struct shear* shear);
void shear_write(struct shear* shear);

void shear_proclens(struct shear* shear, size_t lindex);
void shear_procpair(struct shear* shear, size_t li, size_t si, double cos_search_angle);

#endif

