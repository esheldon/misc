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
#ifdef NO_INCREMENTAL_WRITE
    struct lensums* lensums;
#else
    struct lensum* lensum;
    struct lensum* lensum_tot;
#endif

    // output file pointer. We open at the beginning to make sure we can!
    FILE* fptr;
};

struct shear* shear_init(const char* config_filename);
struct shear* shear_delete(struct shear* shear);

// where we write results before copying to nfs file system
void shear_open_output(struct shear* shear);
FILE* shear_close_output(struct shear* shear);
void shear_cleanup_tempfile(struct shear* shear);
void shear_copy_temp_to_output(struct shear* shear);

void shear_calc(struct shear* shear);

void shear_print_sum(struct shear* shear);
void shear_write_all(struct shear* shear);

void shear_proclens(struct shear* shear, size_t lindex);
void shear_procpair(struct shear* shear, size_t li, size_t si, double cos_search_angle);

#ifdef SDSSMASK
int shear_test_quad(struct lens* l, struct source* s);
#endif

#endif

