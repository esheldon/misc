#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "config.h"

int main(int argc, char *argv[])
{

    enum cfg_status status=0;
    struct cfg *cfg=NULL;
    double dblval=0, *darr=NULL;
    size_t i=0, dsize=0;

    cfg=cfg_read("test.cfg", &status);
    if (status) {
        fprintf(stderr,"Error: %s\n", cfg_status_string(status));
    }

    cfg_print(cfg, stdout);

    printf("\nextracting values\n");
    dblval=cfg_get_double(cfg, "dblval", &status);
    if (status) {
        fprintf(stderr,"Error: %s\n", cfg_status_string(status));
        exit(1);
    }
    printf("    dblval: %lf\n", dblval);

    darr=cfg_get_dblarr(cfg, "darr", &dsize, &status);
    if (status) {
        fprintf(stderr,"Error: %s\n", cfg_status_string(status));
        exit(1);
    }

    for (i=0; i<dsize; i++) {
        printf("    darr[%lu]: %.16g\n", i, darr[i]);
    }


    cfg=cfg_del(cfg);
    return 0;

    /*
    struct cfg_list* cfg_list=NULL;
    enum cfg_status status;
    double dbl=0;
    long lng=0;

    double *darr=NULL;
    size_t darrsize=0;
    long *larr=NULL;
    size_t larrsize=0;

    char **sarr=NULL;
    size_t sarrsize=0;

    char *tmp=NULL;
    char str[80];
    cfg_list= cfg_parse("test.cfg", &status);
    if (CFG_SUCCESS != status) {
        fprintf(stdout,"failure, exiting\n");
        exit(1);
    }
    cfg_print(cfg_list, stdout);

    printf("\n\n");
    dbl = cfg_get_double(cfg_list, "dbl", &status);
    if (status) {
        fprintf(stderr,"Could not get dbl as double: %s\n",cfg_status_string(status));
    } else {
        printf("double dbl: %.16g\n", dbl);
    }
    dbl = cfg_get_double(cfg_list, "lng", &status);
    if (status) {
        fprintf(stderr,"Could not get lng as double: %s\n",cfg_status_string(status));
    } else {
        printf("double lng: %.16g\n", dbl);
    }
    dbl = cfg_get_double(cfg_list, "non-existent", &status);
    if (status) {
        fprintf(stderr,"Could not get non-existent as double: %s\n",cfg_status_string(status));
    } else {
        printf("double non-existent: %.16g\n", dbl);
    }



    lng = cfg_get_long(cfg_list, "lng", &status);
    if (status) {
        fprintf(stderr,"Could not get lng as long: %s\n",cfg_status_string(status));
    } else {
        printf("long lng: %ld\n", lng);
    }

    tmp = cfg_get_string(cfg_list, "str", &status);
    if (status) {
        fprintf(stderr,"Could not get str as string: %s\n",cfg_status_string(status));
    } else {
        printf("string str: '%s'\n", tmp);
    }

    cfg_copy_string(cfg_list, "str", str, 80, &status);
    if (status) {
        fprintf(stderr,"Could not copy str: %s\n",cfg_status_string(status));
    } else {
        printf("string str as copy: '%s'\n", str);
    }

    darr = cfg_get_dblarr(cfg_list, "darr", &darrsize, &status);
    if (status) {
        fprintf(stderr,"Could not get a as double array: %s\n",cfg_status_string(status));
    } else {
        size_t i=0;
        printf("double array: \n");
        for (i=0; i<darrsize; i++) {
            printf("  %.16g\n", darr[i]);
        }
    }

    larr = cfg_get_lonarr(cfg_list, "larr", &larrsize, &status);
    if (status) {
        fprintf(stderr,"Could not get iarr as long array: %s\n",cfg_status_string(status));
    } else {
        size_t i=0;
        printf("long array: \n");
        for (i=0; i<larrsize; i++) {
            printf("  %ld\n", larr[i]);
        }
    }

    sarr = cfg_get_strarr(cfg_list, "sarr", &sarrsize, &status);
    if (status) {
        fprintf(stderr,"Could not get sarr as string array: %s\n",cfg_status_string(status));
    } else {
        size_t i=0;
        printf("string array: \n");
        for (i=0; i<sarrsize; i++) {
            printf("  '%s'\n", sarr[i]);
        }
    }

    cfg_list = cfg_del(cfg_list);
    free(tmp); tmp=NULL;
    free(darr); darr=NULL;
    free(larr); larr=NULL;
    sarr=cfg_strarr_del(sarr, sarrsize);


    return 0;
    */
}
