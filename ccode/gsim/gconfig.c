#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"
#include "gconfig.h"

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

    cfg=cfg_free(cfg);

    if (!gconfig_check(self)) {
        exit(EXIT_FAILURE);
    }
    return self;

}

void gconfig_write(struct gconfig *self, FILE* fobj)
{
    fprintf(fobj,"nrow:       %ld\n", self->nrow);
    fprintf(fobj,"ncol:       %ld\n", self->ncol);
    fprintf(fobj,"noise_type: %s\n", self->noise_type);
    fprintf(fobj,"sky:        %lf\n", self->sky);
    fprintf(fobj,"nsub:       %ld\n", self->nsub);
    fprintf(fobj,"seed:       %ld\n", self->seed);
}


int gconfig_check(struct gconfig *self)
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

    if ( (0 != strcmp(self->noise_type,"poisson")) 
            &
         (0 != strcmp(self->noise_type,"gauss"))) { 

        fprintf(stderr,"bad noise_type: '%s'\n", self->noise_type);
        return 0;
    }

    return 1;

}

