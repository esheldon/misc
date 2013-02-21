#include <stdio.h>
#include <stdlib.h>
#include "config.h"
#include "gconfig.h"

struct gconfig *gconfig_read(const char* filename)
{
    struct gconfig *gconfig=NULL;

    fprintf(stderr,"reading config: %s\n",filename);

    enum cfg_status status=0;
    struct cfg *cfg=cfg_read(filename, &status);
    if (status) {
        fprintf(stderr,"Error: %s\n", cfg_status_string(status));
        exit(EXIT_FAILURE);
    }

    gconfig=calloc(1, sizeof(struct gconfig));
    if (!gconfig) {
        fprintf(stderr,"Error allocating gconfig\n");
        exit(EXIT_FAILURE);
    }

    gconfig->nrow       = cfg_get_long(cfg, "nrow", &status);
    if (status) {
        fprintf(stderr,"Error: %s\n", cfg_status_string(status));
        exit(1);
    }

    gconfig->ncol       = cfg_get_long(cfg, "ncol", &status);
    if (status) {
        fprintf(stderr,"Error: %s\n", cfg_status_string(status));
        exit(1);
    }

    char *noise_type = cfg_get_string(cfg, "noise_type", &status);
    if (status) {
        fprintf(stderr,"Error: %s\n", cfg_status_string(status));
        exit(1);
    }
    strncpy(gconfig->noise_type, noise_type, GCONFIG_STR_SIZE);
    free(noise_type);

    char *ellip_type = cfg_get_string(cfg, "ellip_type", &status);
    if (status) {
        fprintf(stderr,"Error: %s\n", cfg_status_string(status));
        exit(1);
    }
    strncpy(gconfig->ellip_type, ellip_type, GCONFIG_STR_SIZE);
    free(ellip_type);

    gconfig->sky        = cfg_get_double(cfg, "sky", &status);
    if (status) {
        fprintf(stderr,"Error: %s\n", cfg_status_string(status));
        exit(1);
    }

    gconfig->nsub       = cfg_get_long(cfg, "nsub", &status);
    if (status) {
        fprintf(stderr,"Error: %s\n", cfg_status_string(status));
        exit(1);
    }

    gconfig->seed       = cfg_get_long(cfg, "seed", &status);
    if (status) {
        fprintf(stderr,"Error: %s\n", cfg_status_string(status));
        exit(1);
    }

    cfg=cfg_free(cfg);

    return gconfig;

}

void gconfig_write(struct gconfig *self, FILE* fobj)
{
    fprintf(fobj,"nrow:       %ld\n", self->nrow);
    fprintf(fobj,"ncol:       %ld\n", self->ncol);
    fprintf(fobj,"noise_type: %s\n", self->noise_type);
    fprintf(fobj,"ellip_type: %s\n", self->ellip_type);
    fprintf(fobj,"sky:        %lf\n", self->sky);
    fprintf(fobj,"nsub:       %ld\n", self->nsub);
    fprintf(fobj,"seed:       %ld\n", self->seed);
}
