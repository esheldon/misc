#ifndef _HEALPIX_H
#define _HEALPIX_H

#include <stdint.h>
#include "defs.h"
#include "stack.h"

#define NS_MAX 268435456 // 2^28 : largest nside available

struct healpix {
    int64 nside;
    int64 npix;
    int64 ncap;
    double area;
};


/* number of pixels in the map for the given nside */
int64 hpix_npix(int64 nside);

/* area of a pixel in radians^2 */
double hpix_area(int64 nside);

/* allocate a new healpix structure */
struct healpix* hpix_new(int64 nside);
void hpix_delete(struct healpix* hpix);


/*
   renders the pixel number ipix (RING scheme) for a pixel which contains
   a point on a sphere at coordinates theta and phi, given the map
   resolution parameter nside
*/
int64 hpix_eq2pix(const struct healpix* hpix, double ra, double dec);

/*
 
  fill in the list of pixels in RING scheme. pixels are *appended* to plist so
  be sure to run i64stack_resize(plist, 0) or _clear or some such if necessary

*/
void hpix_in_ring(
        const struct healpix* hpix, 
        int64 iz, 
        double phi0, 
        double dphi, 
        struct i64stack* plist);

/*
   returns the ring number in {1, 4*nside-1} from the z coordinate
   returns the ring closest to the z provided
*/
int64 hpix_ring_num(const struct healpix* hpix, double z);

/*
   renders the vector (x,y,z) corresponding to input ra,dec
   North pole is (x,y,z)=(0,0,1)
*/

void hpix_eq2vec(double ra, double dec, double vector[3]);



void hpix_radec_degrees_to_thetaphi_radians(double ra, double dec, double* theta, double* phi);

#endif
