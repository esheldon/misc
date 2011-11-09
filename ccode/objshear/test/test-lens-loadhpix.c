#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "../lens.h"
#include "../stack.h"
#include "../healpix.h"
#include "../cosmo.h"

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("usage: %s lens_filename\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char* fname=argv[1];
    struct lcat* lcat = lcat_read(fname);


    struct cosmo* cosmo = cosmo_new(100, 1, 0.3, 0.7, 0.0);
    lcat_add_da(lcat, cosmo);

    lcat_print_firstlast(lcat);

    struct healpix* hpix = hpix_new(64);

    double rmax=36.0;

    struct i64stack** stackarray = calloc(lcat->size, sizeof(struct i64stack*));
    for (size_t i=0; i<lcat->size; i++) {
        stackarray[i] = i64stack_new(0);

        double search_angle = rmax/lcat->data[i].da;
        hpix_disc_intersect(
                hpix, 
                lcat->data[i].ra, lcat->data[i].dec, 
                search_angle, 
                stackarray[i]);

    }

    printf("sleeping\n");
    sleep(10000);
    lcat=lcat_delete(lcat);
}
