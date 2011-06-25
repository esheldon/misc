#include <stdlib.h>
#include <stdio.h>

#include "shear.h"
#include "config.h"
#include "cosmo.h"
#include "healpix.h"
#include "lens.h"
#include "lensum.h"
#include "source.h"

struct shear* shear_init(const char* config_file) {

    struct shear* shear = calloc(1, sizeof(struct shear));
    if (shear == NULL) {
        printf("Failed to alloc shear struct\n");
        exit(EXIT_FAILURE);
    }

    shear->config=config_read(config_file);

    struct config* config=shear->config;
    printf("config structure:\n");
    config_print(config);


    // open the output file right away to make sure
    // we can
    printf("Opening output file: %s\n", config->output_file);
    shear->fptr = fopen(config->output_file, "w");
    if (shear->fptr == NULL) {
        printf("Could not open output file\n");
        exit(EXIT_FAILURE);
    }


    // now initialize the structures we need
    printf("Initalizing cosmo in flat universe\n");
    int flat=1;
    double omega_k=0;
    shear->cosmo = cosmo_new(config->H0, flat, 
                             config->omega_m, 
                             1-config->omega_m, 
                             omega_k);

    printf("cosmo structure:\n");
    cosmo_print(shear->cosmo);

    printf("Initalizing healpix at nside: %ld\n", config->nside);
    shear->hpix = hpix_new(config->nside);

    // this holds the sums for each lens
    printf("Creating lensum\n");
    shear->lensum=lensum_new(config->nbin);

    // this is a growable stack for holding pixels
    printf("Creating pixel stack\n");
    shear->pixstack = i64stack_new(0);

    // finally read the data
    shear->lcat = lcat_read(config->lens_file);

    printf("Adding Dc to lenses\n");
    lcat_add_dc(shear->cosmo, shear->lcat);
    lcat_print_firstlast(shear->lcat);


    shear->scat = scat_read(config->source_file);

    printf("Adding hpixid to sources\n");
    scat_add_hpixid(shear->hpix, shear->scat);
    printf("Adding revind to scat\n");
    scat_add_rev(shear->hpix, shear->scat);

#ifdef WITH_TRUEZ
    scat_add_dc(shear->cosmo, shear->scat);
#endif

    scat_print_firstlast(shear->scat);

    return shear;

}

struct shear* shear_delete(struct shear* shear) {

    if (shear != NULL) {
        shear->config = config_delete(shear->config);
        shear->lcat   = lcat_delete(shear->lcat);
        shear->scat   = scat_delete(shear->scat);
        shear->hpix   = hpix_delete(shear->hpix);
        shear->cosmo  = cosmo_delete(shear->cosmo);
        shear->pixstack = i64stack_delete(shear->pixstack);
        shear->lensum = lensum_delete(shear->lensum);

        fclose(shear->fptr);
    }
    free(shear);
    return NULL;
}


void shear_calc_bylens(struct shear* shear) {

    int dotstep=500;
    printf("Each dot is %d\n", dotstep);
    for (size_t i=0; i<shear->lcat->size; i++) {
        shear_proclens(shear, i);
        if ( ((i+1) % dotstep) == 0) {
            printf(".");fflush(stdout);
        }
    }
    printf("\nDone\n");
}

void shear_proclens(struct shear* shear, size_t index) {

    struct lens* lens = &shear->lcat->data[index];

    double da = lens->dc/(1+lens->z);
    double search_angle = shear->config->rmax/da;

    hpix_disc_intersect(
            shear->hpix, 
            lens->ra, lens->dec, 
            search_angle, 
            shear->pixstack);
}
