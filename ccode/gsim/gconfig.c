#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fitsio.h>
#include "config.h"
#include "gconfig.h"

static void gconfig_load_wcs(struct gconfig *self, const struct cfg *cfg)
{
    enum cfg_status status=0;

    struct cfg *wcs_cfg = cfg_get_sub(cfg, "wcs", &status);
    if (status) {
        fprintf(stderr,"no wcs found\n");
        return;
    }

    self->has_wcs=1;
    struct simple_wcs *wcs=&self->wcs;

    wcs->cd1_1 = cfg_get_double(wcs_cfg, "cd1_1", &status);
    if (status) {
        fprintf(stderr,"Error getting cd1_1: %s\n", cfg_status_string(status));
        exit(1);
    }
    wcs->cd1_2 = cfg_get_double(wcs_cfg, "cd1_2", &status);
    if (status) {
        fprintf(stderr,"Error getting cd1_2: %s\n", cfg_status_string(status));
        exit(1);
    }
    wcs->cd2_1 = cfg_get_double(wcs_cfg, "cd2_1", &status);
    if (status) {
        fprintf(stderr,"Error getting cd2_1: %s\n", cfg_status_string(status));
        exit(1);
    }
    wcs->cd2_2 = cfg_get_double(wcs_cfg, "cd2_2", &status);
    if (status) {
        fprintf(stderr,"Error getting cd2_2: %s\n", cfg_status_string(status));
        exit(1);
    }

    wcs->crval1 = cfg_get_double(wcs_cfg, "crval1", &status);
    if (status) {
        fprintf(stderr,"Error getting crval1: %s\n", cfg_status_string(status));
        exit(1);
    }
    wcs->crval2 = cfg_get_double(wcs_cfg, "crval2", &status);
    if (status) {
        fprintf(stderr,"Error getting crval2: %s\n", cfg_status_string(status));
        exit(1);
    }

    wcs->crpix1 = cfg_get_double(wcs_cfg, "crpix1", &status);
    if (status) {
        fprintf(stderr,"Error getting crpix1: %s\n", cfg_status_string(status));
        exit(1);
    }
    wcs->crpix2 = cfg_get_double(wcs_cfg, "crpix2", &status);
    if (status) {
        fprintf(stderr,"Error getting crpix2: %s\n", cfg_status_string(status));
        exit(1);
    }

    char *tstr=NULL;

    tstr = cfg_get_string(wcs_cfg, "ctype1", &status);
    if (status) {
        fprintf(stderr,"Error getting ctype1: %s\n", cfg_status_string(status));
        exit(1);
    }
    strncpy(wcs->ctype1, tstr, CTYPE_STRLEN);
    free(tstr); tstr=NULL;

    tstr = cfg_get_string(wcs_cfg, "ctype2", &status);
    if (status) {
        fprintf(stderr,"Error getting ctype2: %s\n", cfg_status_string(status));
        exit(1);
    }
    strncpy(wcs->ctype2, tstr, CTYPE_STRLEN);
    free(tstr); tstr=NULL;


}

struct gconfig *gconfig_read(const char* filename)
{
    struct gconfig *self=NULL;

    fprintf(stderr,"reading config: %s\n",filename);

    enum cfg_status status=0;
    struct cfg *cfg=cfg_read(filename, &status);
    if (status) {
        fprintf(stderr,"Error reading config: %s\n", 
                cfg_status_string(status));
        exit(EXIT_FAILURE);
    }

    self=calloc(1, sizeof(struct gconfig));
    if (!self) {
        fprintf(stderr,"Error allocating gconfig\n");
        exit(EXIT_FAILURE);
    }

    self->nrow = cfg_get_long(cfg, "nrow", &status);
    if (status) {
        fprintf(stderr,"Error getting nrow: %s\n", cfg_status_string(status));
        exit(1);
    }

    self->ncol = cfg_get_long(cfg, "ncol", &status);
    if (status) {
        fprintf(stderr,"Error getting ncol: %s\n", cfg_status_string(status));
        exit(1);
    }

    char *noise_type = cfg_get_string(cfg, "noise_type", &status);
    if (status) {
        fprintf(stderr,"Error getting noise type: %s\n", 
                cfg_status_string(status));
        exit(1);
    }
    strncpy(self->noise_type, noise_type, GCONFIG_STR_SIZE);
    free(noise_type);

    self->sky = cfg_get_double(cfg, "sky", &status);
    if (status) {
        fprintf(stderr,"Error getting sky: %s\n", cfg_status_string(status));
        exit(1);
    }

    self->nsub = cfg_get_long(cfg, "nsub", &status);
    if (status) {
        fprintf(stderr,"Error getting nsub: %s\n", cfg_status_string(status));
        exit(1);
    }

    self->seed = cfg_get_long(cfg, "seed", &status);
    if (status) {
        fprintf(stderr,"Error getting seed: %s\n", cfg_status_string(status));
        exit(1);
    }

    gconfig_load_wcs(self, cfg);

    cfg=cfg_free(cfg);

    if (!gconfig_check(self)) {
        exit(EXIT_FAILURE);
    }
    return self;

}

void gconfig_wcs_write(const struct gconfig *self, FILE* fobj)
{
    const struct simple_wcs *wcs=&self->wcs;
    fprintf(fobj,"wcs = {\n");
    fprintf(fobj,"    cd1_1:      %.16g\n", wcs->cd1_1);
    fprintf(fobj,"    cd1_2:      %.16g\n", wcs->cd1_2);
    fprintf(fobj,"    cd2_1:      %.16g\n", wcs->cd2_1);
    fprintf(fobj,"    cd2_2:      %.16g\n", wcs->cd2_2);

    fprintf(fobj,"    crval1:     %.16g\n", wcs->crval1);
    fprintf(fobj,"    crval2:     %.16g\n", wcs->crval2);

    fprintf(fobj,"    crpix1:     %.16g\n", wcs->crpix1);
    fprintf(fobj,"    crpix2:     %.16g\n", wcs->crpix2);

    fprintf(fobj,"    ctype1:     '%s'\n", wcs->ctype1);
    fprintf(fobj,"    ctype2:     '%s'\n", wcs->ctype2);

    fprintf(fobj,"}\n");
}

void gconfig_write(const struct gconfig *self, FILE* fobj)
{
    fprintf(fobj,"nrow:       %ld\n", self->nrow);
    fprintf(fobj,"ncol:       %ld\n", self->ncol);
    fprintf(fobj,"noise_type: %s\n", self->noise_type);
    fprintf(fobj,"sky:        %lf\n", self->sky);
    fprintf(fobj,"nsub:       %ld\n", self->nsub);
    fprintf(fobj,"seed:       %ld\n", self->seed);

    if (self->has_wcs) {
        gconfig_wcs_write(self, fobj);
    }
}


int gconfig_check(const struct gconfig *self)
{
    if (self->nrow <= 0 || self->ncol <=0) {
        fprintf(stderr,"dims must be > 0, got [%ld, %ld]\n",
                self->nrow,self->ncol);
        return 0;
    }

    if (self->sky < 0) {
        fprintf(stderr,"sky is < 0: %.16g\n", self->sky);
        return 0;
    }
    if (self->nsub < 1) {
        fprintf(stderr,"nsub is < 1: %ld\n", self->nsub);
        return 0;
    }

    if ( (0 != strcasecmp(self->noise_type,"poisson")) 
            &
         (0 != strcasecmp(self->noise_type,"gauss"))
            &
         (0 != strcasecmp(self->noise_type,"none"))) { 

        fprintf(stderr,"bad noise_type: '%s'\n", self->noise_type);
        return 0;
    }

    return 1;

}



static void gconfg_write_wcs2fits(const struct gconfig *self, fitsfile *fits)
{
    int status=0;
    int decimals=-15;


    const struct simple_wcs *wcs=&self->wcs;

    if (fits_update_key_dbl(fits, "cd1_1", wcs->cd1_1, decimals, NULL, &status)) {
        fits_report_error(stderr, status);
        exit(1);
    }
    if (fits_update_key_dbl(fits, "cd1_2", wcs->cd1_2, decimals, NULL, &status)) {
        fits_report_error(stderr, status);
        exit(1);
    }
    if (fits_update_key_dbl(fits, "cd2_1", wcs->cd2_1, decimals, NULL, &status)) {
        fits_report_error(stderr, status);
        exit(1);
    }
    if (fits_update_key_dbl(fits, "cd2_2", wcs->cd2_2, decimals, NULL, &status)) {
        fits_report_error(stderr, status);
        exit(1);
    }


    if (fits_update_key_dbl(fits, "crval1", wcs->crval1, decimals, NULL, &status)) {
        fits_report_error(stderr, status);
        exit(1);
    }
    if (fits_update_key_dbl(fits, "crval2", wcs->crval2, decimals, NULL, &status)) {
        fits_report_error(stderr, status);
        exit(1);
    }

    if (fits_update_key_dbl(fits, "crpix1", wcs->crpix1, decimals, NULL, &status)) {
        fits_report_error(stderr, status);
        exit(1);
    }
    if (fits_update_key_dbl(fits, "crpix2", wcs->crpix2, decimals, NULL, &status)) {
        fits_report_error(stderr, status);
        exit(1);
    }



    if (fits_update_key_str(fits, "ctype1", (char*)wcs->ctype1, NULL, &status)) {
        fits_report_error(stderr, status);
        exit(1);
    }
    if (fits_update_key_str(fits, "ctype2", (char*)wcs->ctype2, NULL, &status)) {
        fits_report_error(stderr, status);
        exit(1);
    }


}
void gconfg_write2fits(const struct gconfig *self, const char* filename)
{
    int status=0;
    fitsfile* fits=NULL;
    int decimals=-15;

    if (fits_open_file(&fits, filename, READWRITE, &status)) {
        fprintf(stderr,"Failed to open file: %s\n", filename);
        fits_report_error(stderr, status);
        exit(EXIT_FAILURE);
    }

    fprintf(stderr,"writing header keywords\n");
    if (fits_update_key_dbl(fits, "sky", self->sky, decimals, "sky in e/s", &status)) {
        fits_report_error(stderr, status);
        exit(1);
    }

    if (fits_update_key_lng(fits, "nsub", self->nsub, "sub-pixel integration in each dimension", &status)) {
        fits_report_error(stderr, status);
        exit(1);
    }
    if (fits_update_key_lng(fits, "seed", self->seed, "seed for random numbers", &status)) {
        fits_report_error(stderr, status);
        exit(1);
    }

    if (fits_update_key_str(fits, "noisetyp", (char*)self->noise_type, "noise type", &status)) {
        fits_report_error(stderr, status);
        exit(1);
    }

    fprintf(stderr,"    writing wcs keywords\n");
    gconfg_write_wcs2fits(self, fits);


    if (fits_close_file(fits, &status)) {
        fits_report_error(stderr, status);
        exit(EXIT_FAILURE);
    }

}
