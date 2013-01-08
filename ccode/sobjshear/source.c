#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "math.h"
#include "source.h"
#include "Vector.h"
#include "log.h"
#include "sdss-survey.h"
#include "sconfig.h"

struct source* source_new(struct sconfig* config) {
    struct source* src = calloc(1,sizeof(struct source));
    if (src == NULL) {
        wlog("Could not allocate struct source\n");
        exit(EXIT_FAILURE);
    }

    src->mask_style = config->mask_style;
    src->scstyle = config->scstyle;

    if (src->scstyle == SCSTYLE_INTERP) {
        src->scinv = f64vector_new(config->nzl);

        if (src->scinv == NULL) {
            wlog("Could not allocate %ld scinv for source\n", config->nzl);
            exit(EXIT_FAILURE);
        }
    }

    return src;
}

// source must already be allocated
int source_read(FILE* stream, struct source* src) {
    size_t i=0;
    double ra_rad,dec_rad;
    int nread=0;
    int nexpect=0;

    nread += fscanf(stream, "%lf %lf %lf %lf %lf %lf %lf", 
            &src->ra, &src->dec, &src->g1, &src->g2, 
            &src->err, &src->mag, &src->R);

    if (src->scstyle == SCSTYLE_INTERP) {
        nexpect = 7+src->scinv->size;
        for (i=0; i<src->scinv->size; i++) {
            nread += fscanf(stream,"%lf", &src->scinv->data[i]);
        }
    } else {
        nexpect = 7+1;
        nread += fscanf(stream,"%lf", &src->z);
    }

    ra_rad = src->ra*D2R;
    dec_rad = src->dec*D2R;
    src->sinra = sin(ra_rad);
    src->cosra = cos(ra_rad);
    src->sindec = sin(dec_rad);
    src->cosdec = cos(dec_rad);

    // add sin(lam),cos(lam),sin(eta),cos(eta)
    if (src->mask_style == MASK_STYLE_SDSS) {
        eq2sdss_sincos(src->ra,src->dec,
                &src->sinlam, &src->coslam,
                &src->sineta, &src->coseta);
    }

    return (nread == nexpect);
}

// filter sources if mag and R were present
int source_filter(struct source *src, struct sconfig *cfg) {
    if (src->mag > cfg->mag_min && src->mag < cfg->mag_max 
        && src->R > cfg->R_min && src->R < cfg->R_max) {
        return 1;
    } else {
        return 0;
    }
}

void source_print(struct source* src) {
    wlog("    ra:     %lf  dec: %lf\n", src->ra, src->dec);
    wlog("    g1:     %lf  g2: %lf\n", src->g1, src->g2);
    wlog("    err:    %lf\n", src->err);
    wlog("    hpixid: %ld\n", src->hpixid);
    wlog("    mag:    %lf\n", src->mag);
    wlog("    R:      %lf\n", src->R);

    if (src->mask_style == MASK_STYLE_SDSS) {
        wlog("    sinlam: %lf\n", src->sinlam);
        wlog("    coslam: %lf\n", src->coslam);
        wlog("    sineta: %lf\n", src->sineta);
        wlog("    coseta: %lf\n", src->coseta);
    }

    if (src->scstyle == SCSTYLE_TRUE) {
        wlog("    z:      %lf\n", src->z);
        wlog("    dc:     %lf\n", src->dc);
    } else {
        size_t nzl = src->zlens->size;
        wlog("    zlens[0]: %lf  szinv[0]: %e\n", 
                src->zlens->data[0], src->scinv->data[0]);
        wlog("    zlens[%ld]: %lf  szinv[%ld]: %e\n", 
                nzl-1, src->zlens->data[nzl-1], nzl-1, src->scinv->data[nzl-1]);
    }
}


// use like this:
//   source = source_delete(source);
struct source* source_delete(struct source* src) {

    if (src != NULL) {

        if (src->scstyle == SCSTYLE_INTERP) {
            if (src->scinv != NULL) {
                src->scinv = f64vector_delete(src->scinv);
            }
        }

        free(src);
    }
    return NULL;
}
