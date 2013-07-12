#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "config.h"

int main(int argc, char *argv[])
{

    enum cfg_status status=0;
    struct cfg *cfg=NULL, *sub=NULL;
    double dblval=0, *darr=NULL, *dempty=NULL;
    long lonval=0, *larr=NULL;
    size_t i=0, dsize=0, dempty_size=0, mixed_size=0;
    char **mixed=NULL;
    char *str1=NULL, *str2=NULL, *name=NULL;

    if (argc > 1) {
        cfg=cfg_read(argv[1], &status); 
    } else {
        cfg=cfg_read("test.cfg", &status); 
    }
    if (status) {
        fprintf(stderr,"Error: %s\n", cfg_status_string(status));
        goto _bail;
    }

    fprintf(stderr,"Printing entire config:\n\n");
    cfg_print(cfg, stdout);

    printf("\nextracting scalar values\n");

    dblval=cfg_get_double(cfg, "dblval", &status);
    if (status) {
        fprintf(stderr,"Error: %s\n", cfg_status_string(status));
        goto _bail;
    }
    printf("    dblval: %lf\n", dblval);

    lonval=cfg_get_long(cfg, "lonval", &status);
    if (status) {
        fprintf(stderr,"Error: %s\n", cfg_status_string(status));
        goto _bail;
    }
    printf("    lonval: %ld\n", lonval);


    str1=cfg_get_string(cfg, "multiline_string", &status);
    if (status) {
        fprintf(stderr,"Error: %s\n", cfg_status_string(status));
        goto _bail;
    }
    printf("    multiline_string: '%s'\n", str1);

    str2=cfg_get_string(cfg, "embed", &status);
    if (status) {
        fprintf(stderr,"Error: %s\n", cfg_status_string(status));
        goto _bail;
    }
    printf("    embed: '%s'\n", str2);


    printf("\nextracting arrays\n");

    darr=cfg_get_dblarr(cfg, "darr", &dsize, &status);
    if (status) {
        fprintf(stderr,"Error: %s\n", cfg_status_string(status));
        goto _bail;
    }
    printf("darr size: %lu\n", dsize);
    for (i=0; i<dsize; i++) {
        printf("    darr[%lu]: %.16g\n", i, darr[i]);
    }

    dempty=cfg_get_dblarr(cfg, "empty", &dempty_size, &status);
    if (status) {
        fprintf(stderr,"Error: %s\n", cfg_status_string(status));
        goto _bail;
    }
    printf("dempty size: %lu\n", dempty_size);
    for (i=0; i<dempty_size; i++) {
        printf("    empty[%lu]: %.16g\n", i, dempty[i]);
    }

    larr=cfg_get_lonarr(cfg, "larr", &dsize, &status);
    if (status) {
        fprintf(stderr,"Error: %s\n", cfg_status_string(status));
        goto _bail;
    }
    printf("larr size: %lu\n", dsize);
    for (i=0; i<dsize; i++) {
        printf("    larr[%lu]: %ld\n", i, larr[i]);
    }


    mixed=cfg_get_strarr(cfg, "mixed", &mixed_size, &status);
    if (status) {
        fprintf(stderr,"Error: %s\n", cfg_status_string(status));
        goto _bail;
    }
    printf("mixed size: %lu\n", mixed_size);
    for (i=0; i<mixed_size; i++) {
        printf("    mixed[%lu]: '%s'\n", i, mixed[i]);
    }


    printf("\ngetting and printing sub-config 'state'\n");
    sub = cfg_get_sub(cfg, "state", &status);
    if (status) {
        fprintf(stderr,"Error: %s\n", cfg_status_string(status));
        goto _bail;
    }
    cfg_print(sub,stdout);

    name=cfg_get_string(sub, "name", &status);
    if (status) {
        fprintf(stderr,"Error: %s\n", cfg_status_string(status));
        goto _bail;
    }
    printf("name from sub: '%s'\n", name);


    printf("\ntrying to get a non-existent field\n");
    lonval=cfg_get_long(cfg, "crap", &status);
    if (status) {
        fprintf(stderr,"Error: %s\n", cfg_status_string(status));
        fprintf(stderr,"As expected\n");
    }

_bail:

    free(darr); darr=NULL;
    free(dempty); dempty=NULL;
    free(larr); larr=NULL;
    mixed=cfg_strarr_free(mixed, mixed_size);
    free(str1); str1=NULL;
    free(str2); str2=NULL;
    free(name); name=NULL;
    cfg=cfg_free(cfg);
    return 0;

}
