#include <stdlib.h>
#include <stdio.h>

#include "mangle.h"
#include "defs.h"
#include "point.h"

int main(int argc, char** argv)
{
    const char* filename=NULL;
    struct MangleMask* mask=NULL;
    struct Point pt;
    int64 polyid=0;
    double weight=0, ra=0, dec=0;

    if (argc < 2) {
        printf("usage: ./testrand mask_file \n"
               "    input is read from stdin as columns\n"
               "        ra dec\n"
               "    output is written to stdout as columns\n"
               "        ra dec polyid weight\n"
               "    It is assumed the polygons are balkanized\n"
               "    and snapped so the polyid is unique\n");
        exit(EXIT_FAILURE);
    }

    filename=argv[1];

    mask=mangle_new();

    mangle_read(mask, filename);
    mangle_print(stderr,mask,1);

    while (2==scanf("%lf %lf", &ra, &dec)) {
        point_set_from_radec(&pt, ra, dec);
        if (!mangle_polyid_and_weight(mask,&pt,&polyid,&weight)) {
            exit(EXIT_FAILURE);
        }
        printf("%.16g %.16g %ld %.16g\n", ra, dec, polyid, weight);
    }

    // if using in a library, call this
    //mangle_free(mask);
}
