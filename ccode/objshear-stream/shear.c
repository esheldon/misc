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
#include "log.h"

#ifdef SDSSMASK
#include "sdss-survey.h"
#endif

struct shear* shear_init(const char* config_file) {

    struct shear* shear = calloc(1, sizeof(struct shear));
    if (shear == NULL) {
        wlog("Failed to alloc shear struct\n");
        exit(EXIT_FAILURE);
    }

    shear->config=config_read(config_file);

    struct config* config=shear->config;
    wlog("config structure:\n");
    config_print(config);

    // now initialize the structures we need
    wlog("Initalizing cosmo in flat universe\n");
    int flat=1;
    double omega_k=0;
    shear->cosmo = cosmo_new(config->H0, flat, 
                             config->omega_m, 
                             1-config->omega_m, 
                             omega_k);

    wlog("cosmo structure:\n");
    cosmo_print(shear->cosmo);

    wlog("Initalizing healpix at nside: %ld\n", config->nside);
    shear->hpix = hpix_new(config->nside);

    // finally read the data
    shear->lcat = lcat_read(config->lens_file);

    wlog("Adding Da to lenses\n");
    lcat_add_da(shear->lcat, shear->cosmo);
    lcat_add_search_angle(shear->lcat, config->rmax);
    wlog("Intersecting all lenses with healpix at rmax: %lf\n", config->rmax);
    lcat_disc_intersect(shear->lcat, config->nside, config->rmax);

    lcat_print_firstlast(shear->lcat);

    // this holds the sums for each lens
    shear->lensums = lensums_new(shear->lcat->size, config->nbin);
    for (size_t i=0; i<shear->lensums->size; i++) {
        shear->lensums->data[i].zindex = shear->lcat->data[i].zindex;
    }


#ifndef WITH_TRUEZ
    // interpolation region
    double* zl=config->zl->data;
    int64 nzl=config->nzl;
    shear->min_zlens = zl[0];
    shear->max_zlens = zl[nzl-1];
    shear->min_zlens = fmax(shear->min_zlens, MIN_ZLENS);
#else
    shear->min_zlens = 0;
    shear->max_zlens = 9999;
#endif



    return shear;

}

void shear_process_source(struct shear* self, struct source* src) {
    src->hpixid = hpix_eq2pix(self->hpix, src->ra, src->dec);
    struct lens* lens= &self->lcat->data[0];
    struct lensum* lensum = &self->lensums->data[0];
    for (size_t i=0; i<self->lcat->size; i++) {

        if (lens->z >= self->min_zlens 
                && lens->z <= self->max_zlens) {
                //&& src->hpixid >= lens->hpix->data[0]
                //&& src->hpixid <= lens->hpix->data[lens->hpix->size-1]) {

            lensum->totpairs++;
        }


        /*
        if (shear_trylens(self, src, lens)) {
            lensum->totpairs++;
            //shear_procpair(self, src, lens, lensum);
        }
        */
        lens++;
        lensum++;
    }
}

int shear_trylens(struct shear* self, struct source* src, struct lens* lens) {
    return 1;
    //int64* hpix_ptr=NULL;
    if (lens->z >= self->min_zlens 
            && lens->z <= self->max_zlens
            && src->hpixid >= lens->hpix->data[0]
            && src->hpixid <= lens->hpix->data[lens->hpix->size-1]) {

        return 1;

        // now make sure the actual pixel value is found
        /*
        if (lens->rev->data[src->hpixid] != lens->rev->data[src->hpixid+1]) {
            return 1;
        }
        */
        /*
        hpix_ptr = i64stack_find(lens->hpix, src->hpixid);
        if (hpix_ptr != NULL) {
            return 1;
        }
        */
    }
    return 0;
}


void shear_procpair(struct shear* self, 
                    struct source* src, 
                    struct lens* lens, 
                    struct lensum* lensum) {

    //struct config* config=self->config;
    
    double cosphi, cosradiff, sinradiff, theta;
    double phi, arg;//, cos2theta, sin2theta;
    //double scinv;

#ifdef SDSSMASK
    // make sure object is in a pair of unmasked adjacent quadrants
    if (!shear_test_quad(lens, src)) {
        return;
    }
#endif

    cosradiff = src->cosra*lens->cosra + src->sinra*lens->sinra;
    cosphi = lens->sindec*src->sindec + lens->cosdec*src->cosdec*cosradiff;

    if (cosphi > lens->cos_search_angle) {
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
        /*
        cos2theta = cos(2*theta);
        sin2theta = sin(2*theta);
        */

    }
}

/*
void shear_calc(struct shear* shear) {

#ifndef WITH_TRUEZ
    // interpolation region
    double minz = shear->scat->min_zlens;
    double maxz = shear->scat->max_zlens;
#else
    double minz=0;
    double maxz=9999;
#endif

    wlog("printing one dot every %d lenses\n", LENSPERDOT);

#ifndef NO_INCREMENTAL_WRITE
    lensums_write_header(shear->lcat->size, 
                         shear->lensum->nbin, 
                         shear->fptr);
#endif

    size_t nlens = shear->lcat->size;
    //nlens = 10000;
    for (size_t i=0; i<nlens; i++) {
        if ( ((i+1) % LENSPERCHUNK) == 0) {
            wlog("\n%ld/%ld  (%0.1f%%)\n", i+1, nlens, 100.*(float)(i+1)/nlens);
            fflush(stdout);
        }
        if ( (i % LENSPERDOT) == 0) {
            wlog(".");
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
    wlog("\n");
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

*/

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




void shear_print_sum(struct shear* self) {
    wlog("Total sums:\n\n");

    lensums_print_sum(self->lensums);

}

// this is for when we haven't written the file line by line[:w
void shear_write_all(struct shear* self, FILE* stream) {
    lensums_write(self->lensums, stream);
}

struct shear* shear_delete(struct shear* self) {

    if (self != NULL) {

        self->config   = config_delete(self->config);
        self->lcat     = lcat_delete(self->lcat);
        self->hpix     = hpix_delete(self->hpix);
        self->cosmo    = cosmo_delete(self->cosmo);
        self->lensums  = lensums_delete(self->lensums);

    }
    free(self);
    return NULL;
}


