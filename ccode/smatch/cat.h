#ifndef _CAT_H
#define _CAT_H

#include <stdint.h>
#include "point.h"
#include "healpix.h"
#include "point_hash.h"
#include "match.h"
#include "VEC.h"

struct cat {
    size_t size;
    struct point* pts;
    struct healpix* hpix;
    struct point_hash* pthash;
};

struct cat* cat_new(size_t n, int64 nside);
struct cat* read_cat(const char* fname, 
                     int64 nside, 
                     double radius_arcsec, 
                     int verbose);

struct cat* _cat_read(FILE* fptr, 
               size_t nlines,
               int64 nside,
               double radius_arcsec,
               int verbose);

void cat_match(struct cat* self, 
               double ra, 
               double dec,
               VEC(Match) matches);
 
int _cat_get_radius(double radius_arcsec, 
                    double* radius_radians,
                    double* cos_radius_global);

void repeat_char(char c, int n);
void incr_bar(size_t this_index, 
              size_t ntot, 
              size_t ntoprint, 
              char c);
#endif
