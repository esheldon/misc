#ifndef _HEALPIX_H
#define _HEALPIX_H

#include <stdint.h>
#include "defs.h"

#define NS_MAX 268435456 // 2^28 : largest nside available

/* number of pixels in the map for the given nside */
int64 hpix_npix(int64 nside);

/* area in radians^2 */
double hpix_pixarea(int64 nside);

/*
   renders the pixel number ipix (RING scheme) for a pixel which contains
   a point on a sphere at coordinates theta and phi, given the map
   resolution parameter nside
*/
int64 hpix_eq2pix(int64 nside, double ra, double dec);


/*
   renders the vector (x,y,z) corresponding to angles

   ra gets converted to phi:
       (longitude measured eastward, in radians [0,2*pi]
   dec gets converted to theta:
       (co-latitude measured from North pole, in [0,Pi] radians)

   North pole is (x,y,z)=(0,0,1)
*/

void hpix_eq2vec(double ra, double dec, double vector[3]);

/*
    !=======================================================================
    ! ring = ring_num(nside, z)
    !     returns the ring number in {1, 4*nside-1}
    !     from the z coordinate
    ! returns the ring closest to the z provided
    !
    !=======================================================================
*/

int64 hpix_ring_num(int64 nside, double z);

void hpix_radec_degrees_to_thetaphi_radians(double ra, double dec, double* theta, double* phi);

#endif
