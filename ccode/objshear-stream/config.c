#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "config.h"
#include "Vector.h"
#include "log.h"

const char* get_config_filename(int argc, char** argv) {
    const char* config_file=NULL;
    if (argc >= 2) {
        config_file=argv[1];
    } else {
        config_file=getenv("CONFIG_FILE");
    }

    return config_file;
}

struct config* config_read(const char* filename) {
    wlog("Reading config from %s\n", filename);
    FILE* stream=fopen(filename,"r");
    if (stream==NULL) {
        wlog("Could not open file: %s\n", filename);
        exit(EXIT_FAILURE);
    }

    struct config* c=calloc(1, sizeof(struct config));
    c->zl=NULL;

    char key[255];
    fscanf(stream, "%s %s", key, c->lens_file);
    fscanf(stream, "%s %lf", key, &c->H0);
    fscanf(stream, "%s %lf", key, &c->omega_m);
    fscanf(stream, "%s %ld", key, &c->npts);
    fscanf(stream, "%s %ld", key, &c->nside);
    fscanf(stream, "%s %ld", key, &c->sigmacrit_style);
    fscanf(stream, "%s %ld", key, &c->nbin);
    fscanf(stream, "%s %lf", key, &c->rmin);
    fscanf(stream, "%s %lf", key, &c->rmax);
    if (c->sigmacrit_style == 2) {
        size_t i;
        fscanf(stream, "%s %lu", key, &c->nzl);
        c->zl = f64vector_new(c->nzl);
        // this is the zlvals keyword
        fscanf(stream," %s ", key);
        for (i=0; i<c->zl->size; i++) {
            fscanf(stream, "%lf", &c->zl->data[i]);
        }
    }

    c->log_rmin = log10(c->rmin);
    c->log_rmax = log10(c->rmax);
    c->log_binsize = (c->log_rmax - c->log_rmin)/c->nbin;

    fclose(stream);

    return c;
}

// usage:  config=config_delete(config);
struct config* config_delete(struct config* config) {
    if (config != NULL) {
        free(config->zl);
    }
    free(config);
    return NULL;
}

void config_print(struct config* c) {
    wlog("    lens_file:    %s\n", c->lens_file);
    wlog("    H0:           %lf\n", c->H0);
    wlog("    omega_m:      %lf\n", c->omega_m);
    wlog("    npts:         %ld\n", c->npts);
    wlog("    nside:        %ld\n", c->nside);
    wlog("    scrit style:  %ld\n", c->sigmacrit_style);
    wlog("    nbin:         %ld\n", c->nbin);
    wlog("    rmin:         %lf\n", c->rmin);
    wlog("    rmax:         %lf\n", c->rmax);
    wlog("    log(rmin):    %lf\n", c->log_rmin);
    wlog("    log(rmax):    %lf\n", c->log_rmax);
    wlog("    log(binsize): %lf\n", c->log_binsize);
    if (c->zl != NULL) {
        size_t i;
        wlog("    zlvals[%lu]:", c->zl->size);
        for (i=0; i<c->zl->size; i++) {
            if ((i % 10) == 0) {
                wlog("\n        ");
            }
            wlog("%lf ", c->zl->data[i]);
        }
        wlog("\n");
    }
}
