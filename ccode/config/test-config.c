#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>

#include "config.h"

int main(int argc, char *argv[])
{

    struct cfg_list* cfg_list=NULL;
    enum cfg_status_code status;
    double y=0;
    long x=0;

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
    y = cfg_get_double(cfg_list, "y", &status);
    if (status) {
        fprintf(stderr,"Could not get y as double: %s\n",cfg_status_string(status));
    } else {
        printf("double y: %.16g\n", y);
    }
    y = cfg_get_double(cfg_list, "x", &status);
    if (status) {
        fprintf(stderr,"Could not get y as double: %s\n",cfg_status_string(status));
    } else {
        printf("double y: %.16g\n", y);
    }
    y = cfg_get_double(cfg_list, "non-existent", &status);
    if (status) {
        fprintf(stderr,"Could not get non-existent as double: %s\n",cfg_status_string(status));
    } else {
        printf("double non-existent: %.16g\n", y);
    }



    x = cfg_get_long(cfg_list, "x", &status);
    if (status) {
        fprintf(stderr,"Could not get x as long: %s\n",cfg_status_string(status));
    } else {
        printf("long x: %ld\n", x);
    }

    tmp = cfg_get_string(cfg_list, "s", &status);
    if (status) {
        fprintf(stderr,"Could not get s as string: %s\n",cfg_status_string(status));
    } else {
        printf("string s: '%s'\n", tmp);
    }

    cfg_copy_string(cfg_list, "s", str, 80, &status);
    if (status) {
        fprintf(stderr,"Could not copy s: %s\n",cfg_status_string(status));
    } else {
        printf("string s as copy: '%s'\n", str);
    }

    darr = cfg_get_dblarr(cfg_list, "a", &darrsize, &status);
    if (status) {
        fprintf(stderr,"Could not get a as double array: %s\n",cfg_status_string(status));
    } else {
        size_t i=0;
        printf("double array: \n");
        for (i=0; i<darrsize; i++) {
            printf("  %.16g\n", darr[i]);
        }
    }

    larr = cfg_get_lonarr(cfg_list, "iarr", &larrsize, &status);
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



    /*
    y = cfg_get_dbl
    y = cfg_get_lon
    y = cfg_get_str
    y = cfg_get_dblarr
    y = cfg_get_lonarr
    y = cfg_get_strarr


    y = cfg_dbl
    y = cfg_lon
    y = cfg_str
    y = cfg_dblarr
    y = cfg_lonarr
    y = cfg_strarr
    */

    //fprintf(stderr,"main: freeing cfg list\n");
    cfg_list = cfg_list_del(cfg_list);
    free(tmp);
    free(darr);
    free(larr);
    sarr=cfg_strarr_del(sarr, sarrsize);


    return 0;
}
