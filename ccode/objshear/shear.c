#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <unistd.h>

#include "defs.h"
#include "shear.h"
#include "config.h"
#include "cosmo.h"
#include "healpix.h"
#include "lens.h"
#include "lensum.h"
#include "source.h"
#include "interp.h"

#ifdef SDSSMASK
#include "sdss-survey.h"
#endif

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

    // the temporary output file where we write everything locally, then copy
    // to the nfs file system after
    shear_open_output(shear);

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


    // this is a growable stack for holding pixels
    printf("Creating pixel stack\n");
    shear->pixstack = i64stack_new(0);
    //shear->pixstack->realloc_multval = 1.1;

    // finally read the data
    shear->lcat = lcat_read(config->lens_file);



    printf("Adding Dc to lenses\n");
    lcat_add_da(shear->lcat, shear->cosmo);
    //lcat_print_firstlast(shear->lcat);
    lcat_print_one(shear->lcat, shear->lcat->size-1);


    shear->scat = scat_read(config->source_file);

    printf("Adding hpixid to sources\n");
    scat_add_hpixid(shear->scat, shear->hpix);
    printf("Adding revind to scat\n");
    scat_add_rev(shear->scat, shear->hpix);


#ifdef WITH_TRUEZ
    scat_add_dc(shear->scat, shear->cosmo);
#endif

    //scat_print_firstlast(shear->scat);
    scat_print_one(shear->scat, shear->scat->size-1);

    //sleep(1000);


#ifdef NO_INCREMENTAL_WRITE
    // this holds the sums for each lens
    shear->lensums = lensums_new(shear->lcat->size, config->nbin);
    for (size_t i=0; i<shear->lensums->size; i++) {
        shear->lensums->data[i].zindex = shear->lcat->data[i].zindex;
    }
#else
    shear->lensum = lensum_new(config->nbin);
    shear->lensum_tot = lensum_new(config->nbin);
#endif

    return shear;

}

void shear_calc(struct shear* shear) {

#ifndef WITH_TRUEZ
    // interpolation region
    double minz = shear->scat->min_zlens;
    double maxz = shear->scat->max_zlens;
#else
    double minz=0;
    double maxz=9999;
#endif

    printf("printing one dot every %d lenses\n", LENSPERDOT);

#ifndef NO_INCREMENTAL_WRITE
    lensums_write_header(shear->lcat->size, 
                         shear->lensum->nbin, 
                         shear->fptr);
#endif

    size_t nlens = shear->lcat->size;
    //nlens = 10000;
    for (size_t i=0; i<nlens; i++) {
        if ( ((i+1) % LENSPERCHUNK) == 0) {
            printf("\n%ld/%ld  (%0.1f%%)\n", i+1, nlens, 100.*(float)(i+1)/nlens);
            fflush(stdout);
        }
        if ( (i % LENSPERDOT) == 0) {
            printf(".");
            fflush(stdout);
        }


#ifndef NO_INCREMENTAL_WRITE
        lensum_clear(shear->lensum);
        shear->lensum->zindex = shear->lcat->data[i].zindex;
#endif

        double z = shear->lcat->data[i].z;
        if (z >= minz && z <= maxz && z > MIN_ZLENS) {
            shear_proclens(shear, i);
        } 

#ifndef NO_INCREMENTAL_WRITE
        // always write out this lensum
        lensum_write(shear->lensum, shear->fptr);
        // keep accumulation of statistics
        lensum_add(shear->lensum_tot, shear->lensum);
#endif

    }
    printf("\n");
}



void shear_proclens(struct shear* shear, size_t lindex) {

    struct lens* lens = &shear->lcat->data[lindex];
    struct scat* scat = shear->scat;
    struct szvector* rev = scat->rev;

    double da = lens->da;

    double search_angle = shear->config->rmax/da;
    double cos_search_angle = cos(search_angle);

    struct i64stack* pixstack = shear->pixstack;


    hpix_disc_intersect(
            shear->hpix, 
            lens->ra, lens->dec, 
            search_angle, 
            pixstack);

    for (size_t i=0; i<pixstack->size; i++) {
        int64 pix = pixstack->data[i];

        if ( pix >= scat->minpix && pix <= scat->maxpix) {
            int64 ipix=pix-scat->minpix;
            size_t nsrc = rev->data[ipix+1] - rev->data[ipix];
            for (size_t j=0; j<nsrc; j++) {

                size_t sindex=rev->data[ rev->data[ipix]+j ];
                assert(sindex < scat->size);
                assert(scat->data[sindex].hpixid == pix);
                shear_procpair(shear, lindex, sindex, cos_search_angle);

            } // sources
        } // pix in range for sources?
    }


}

void shear_procpair(struct shear* shear, size_t li, size_t si, double cos_search_angle) {
    struct lens* lens = &shear->lcat->data[li];
    struct source* src = &shear->scat->data[si];
    struct config* config=shear->config;

#ifdef NO_INCREMENTAL_WRITE
    struct lensum* lensum = &shear->lensums->data[li];
#else
    struct lensum* lensum = shear->lensum;
#endif

    double cosradiff, sinradiff, cosphi, theta;
    double phi, cos2theta, sin2theta, arg;
    double scinv;

#ifdef SDSSMASK
    // make sure object is in a pair of unmasked adjacent quadrants
    if (!shear_test_quad(lens, src)) {
        return;
    }
#endif

    cosradiff = src->cosra*lens->cosra + src->sinra*lens->sinra;
    cosphi = lens->sindec*src->sindec + lens->cosdec*src->cosdec*cosradiff;

    if (cosphi > cos_search_angle) {
        if (cosphi > 1.0) {
            cosphi = 1.0;
        } else if (cosphi < -1.0) {
            cosphi = -1.0;
        }
        phi = acos(cosphi);

        // this is sin(sra-lra), note sign
        sinradiff = src->sinra*lens->cosra - src->cosra*lens->sinra;

        arg = lens->sindec*cosradiff - lens->cosdec*src->sindec/src->cosdec;
        theta = atan2(sinradiff, arg) - M_PI_2;

        // these two calls are a significant fraction of cpu usage
        cos2theta = cos(2*theta);
        sin2theta = sin(2*theta);

        // note we already checked if lens z was in our interpolation range
#ifndef WITH_TRUEZ
        scinv = f64interplin(src->zlens, src->scinv, lens->z);
#else
        double dcl = lens->da*(1.+lens->z);
        scinv = scinv_pre(lens->z, dcl, src->dc);
#endif

        if (scinv > 0) {
            double r, logr;
            int rbin;

            r = phi*lens->da;
            logr = log10(r);

            rbin = (int)( (logr-config->log_rmin)/config->log_binsize );

            if (rbin >= 0 && rbin < config->nbin) {
                double scinv2, gamma1, gamma2, eweight, weight, err2;

                err2 = src->err*src->err;
                scinv2 = scinv*scinv;

                eweight = 1./(GSN2 + err2);
                weight = scinv2*eweight;

                gamma1 = -(src->g1*cos2theta + src->g2*sin2theta);
                gamma2 =  (src->g1*sin2theta - src->g2*cos2theta);

                lensum->weight += weight;
                lensum->totpairs += 1;
                lensum->npair[rbin] += 1;

                lensum->wsum[rbin] += weight;
                lensum->dsum[rbin] += weight*gamma1/scinv;
                lensum->osum[rbin] += weight*gamma2/scinv;

                lensum->rsum[rbin] += r;

                // calculating Ssh, shear polarizability
                // factors of two cancel in both of these
                double f_e = err2*eweight;
                double f_sn = GSN2*eweight;

                // coefficients (p 596 Bern02) 
                // there is a k1*e^2/2 in Bern02 because
                // its the total ellipticity he is using

                double k0 = f_e*GSN2*4;  // factor of (1/2)^2 does not cancel here, 4 converts to shapenoise
                double k1 = f_sn*f_sn;

                // Factors of two don't cancel, need 2*2 for gamma instead of shape
                double F = 1. - k0 - k1*gamma1*gamma1*4;

                // get correction ssh using l->sshsum/l->weight
                lensum->sshsum   += weight*F;

            }
        }
    }
}



/*
 * Make sure the source is in an acceptable quadrant for this lens
 */
#ifdef SDSSMASK
int shear_test_quad(struct lens* l, struct source* s) {
    return test_quad_sincos(l->maskflags,
                            l->sinlam, l->coslam,
                            l->sineta, l->coseta,
                            s->sinlam, s->coslam,
                            s->sineta, s->coseta);
}
#endif




void shear_open_output(struct shear* shear) {
    printf("Opening output file: %s\n", shear->config->output_file);
    shear->fptr = fopen(shear->config->output_file, "w");
    if (shear->fptr == NULL) {
        printf("Could not open temp file\n");
        exit(EXIT_FAILURE);
    }
}
FILE* shear_close_output(struct shear* shear) {
    if (shear->fptr != NULL) {
        // remember, this also flushes
        if (0 != fclose(shear->fptr)) {
            printf("Could not close temp file\n");
            exit(EXIT_FAILURE);
        }
    }
    return NULL;
}
void shear_cleanup_tempfile(struct shear* shear) {
    printf("Deleting temporary file: %s\n", shear->config->temp_file);
    fflush(stdout);
    if (0 != unlink(shear->config->temp_file)) {
        printf("Failed to unlink file %s\n", shear->config->temp_file);
        exit(EXIT_FAILURE);
    }
}

void shear_copy_temp_to_output(struct shear* shear) {

    /*
     * we want this buffer to be a multiple of the page size because a page is
     * the smallest unit of data for both memory allocation and transfer
     * between memory and the hard drive
     * 
     * this is two pages on the intel (amd64) machines we have on the clulster
     */

    int page_size = sysconf(_SC_PAGESIZE);
    char* buffer = calloc(2*page_size,sizeof(char));

    printf("Copying from temp file \n    %s\nto output\n    %s\n", 
           shear->config->temp_file, shear->config->output_file);
    fflush(stdout);

    FILE* srcfile = fopen(shear->config->temp_file, "rb");
    if (srcfile == NULL) {
        printf("Could not open temp file\n");
        exit(EXIT_FAILURE);
    }

    FILE* destfile = fopen(shear->config->output_file, "wb");
    if (destfile == NULL) {
        printf("Could not open outputfile\n");
        exit(EXIT_FAILURE);
    }

    int bytes;

    int ret=0;
    while((bytes = fread(buffer, 1, sizeof(buffer), srcfile)) > 0) {
        ret=fwrite(buffer, 1, bytes, destfile);
        if (ret != bytes) {
            printf("Expected to write %d bytes but wrote only %d\n", bytes, ret);
            exit(EXIT_FAILURE);
        }
    }

    fclose(srcfile);
    fclose(destfile);
    free(buffer);


}





void shear_print_sum(struct shear* shear) {
    printf("Total sums:\n\n");

#ifdef NO_INCREMENTAL_WRITE
    lensums_print_sum(shear->lensums);
#else
    lensum_print(shear->lensum_tot);
#endif

}

// this is for when we haven't written the file line by line[:w
void shear_write_all(struct shear* shear) {

    printf("\nWriting lensums to %s\n", shear->config->output_file);

#ifdef NO_INCREMENTAL_WRITE
    lensums_write(shear->lensums, shear->fptr);
#endif

}


struct shear* shear_delete(struct shear* shear) {

    if (shear != NULL) {
        shear->fptr = shear_close_output(shear);

        shear->config   = config_delete(shear->config);
        shear->lcat     = lcat_delete(shear->lcat);
        shear->scat     = scat_delete(shear->scat);
        shear->hpix     = hpix_delete(shear->hpix);
        shear->cosmo    = cosmo_delete(shear->cosmo);
        shear->pixstack = i64stack_delete(shear->pixstack);
#ifdef NO_INCREMENTAL_WRITE
        shear->lensums  = lensums_delete(shear->lensums);
#else
        shear->lensum  = lensum_delete(shear->lensum);
        shear->lensum_tot  = lensum_delete(shear->lensum_tot);
#endif

    }
    free(shear);
    return NULL;
}


