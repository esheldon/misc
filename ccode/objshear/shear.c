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

    // finally read the data
    shear->lcat = lcat_read(config->lens_file);

    printf("Adding Dc to lenses\n");
    lcat_add_dc(shear->cosmo, shear->lcat);
    lcat_print_firstlast(shear->lcat);


    shear->scat = scat_read(config->source_file);

    scat_add_hpixid(shear->hpix, shear->scat);

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
        shear->lensum = lensum_delete(shear->lensum);

        fclose(shear->fptr);
    }
    free(shear);
    return NULL;
}


