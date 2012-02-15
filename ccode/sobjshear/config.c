#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "config.h"
#include "Vector.h"
#include "log.h"

#ifdef HDFS
#include "hdfs_lines.h"
#endif



struct config* config_read(const char* url) {

#ifdef HDFS
    // if compiled with hdfs, and begins with hdfs:// then read as an hdfs file
    if (is_in_hdfs(url)) {
        return hdfs_config_read(url);
    }
#endif

    wlog("Reading config from %s\n", url);

    FILE* stream=fopen(url,"r");
    if (stream==NULL) {
        wlog("Could not open url: %s\n", url);
        exit(EXIT_FAILURE);
    }

    struct config* c=calloc(1, sizeof(struct config));
    c->zl=NULL;

    char key[CONFIG_KEYSZ];
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

#ifdef HDFS

struct config* hdfs_config_read(const char* url) {

    hdfsFS fs;
    size_t lbsz=255;
    char* lbuf=calloc(lbsz, sizeof(char));

    struct config* c=calloc(1, sizeof(struct config));
    fs = hdfs_connect();

    wlog("Reading config from %s\n", url);

    tSize file_buffsize=1024;
    hdfsFile hf = hdfs_open(fs, url, O_RDONLY, file_buffsize);


    c->zl=NULL;

    char key[CONFIG_KEYSZ];

    hdfs_getline(hf, &lbuf, &lbsz); sscanf(lbuf, "%s %lf", key, &c->H0);
    hdfs_getline(hf, &lbuf, &lbsz); sscanf(lbuf, "%s %lf", key, &c->omega_m);
    hdfs_getline(hf, &lbuf, &lbsz); sscanf(lbuf, "%s %ld", key, &c->npts);
    hdfs_getline(hf, &lbuf, &lbsz); sscanf(lbuf, "%s %ld", key, &c->nside);
    hdfs_getline(hf, &lbuf, &lbsz); sscanf(lbuf, "%s %ld", key, &c->sigmacrit_style);
    hdfs_getline(hf, &lbuf, &lbsz); sscanf(lbuf, "%s %ld", key, &c->nbin);
    hdfs_getline(hf, &lbuf, &lbsz); sscanf(lbuf, "%s %lf", key, &c->rmin);
    hdfs_getline(hf, &lbuf, &lbsz); sscanf(lbuf, "%s %lf", key, &c->rmax);

    if (c->sigmacrit_style == 2) {
        int nread=0;
        char* lptr;

        hdfs_getline(hf, &lbuf, &lbsz); sscanf(lbuf, "%s %lu", key, &c->nzl);
        c->zl = f64vector_new(c->nzl);

        // this is the zlvals keyword
        // note space, that tells it to skip white space
        // reading until tab space or newline (although
        // we don't expect keywords alone on a line)
        hdfs_getline(hf, &lbuf, &lbsz);
        lptr = lbuf;
        sscanf(lptr, " %49[^\t \n]%n", key, &nread);

        // after each read, skip what we read plus delimiter
        lptr += nread+1;

        for (size_t i=0; i<c->zl->size; i++) {
            sscanf(lptr, "%lf%n", &c->zl->data[i], &nread);
            lptr += nread+1;
        }
    }

    c->log_rmin = log10(c->rmin);
    c->log_rmax = log10(c->rmax);
    c->log_binsize = (c->log_rmax - c->log_rmin)/c->nbin;

    hdfsCloseFile(fs, hf);
    hdfsDisconnect(fs);

    free(lbuf);

    return c;
}


#endif



// usage:  config=config_delete(config);
struct config* config_delete(struct config* self) {
    if (self != NULL) {
        free(self->zl);
    }
    free(self);
    return NULL;
}

void config_print(struct config* c) {
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
