#include <math.h>
#include "gcirc.h"
#include "defs.h"

/* lon lat in degrees.
   dis and theta will be in radians
  
   this code is slower than it needs to be due to heavy
   use of trig.  Can speed up by doing this:
     coslat = cos(lat);
     sinlat = sqrt(1.-coslat*coslat);
     if (lat < 0.) sinlat *= -1;
   
   also, since the same sources are often used many times,
   you can save a lot by pre-computing the sin(lat) and
   other things.
  
   to match faster code in shearlib, lon2,lat2 should be
   the sources because of lon2-lon1 in londiff: changes sign of theta
*/
void gcirc(double lon1, double lat1, 
           double lon2, double lat2, 
           double* dis, double* theta) {

    static const double d2r = 0.017453292519943295;
    double lat_rad1, lat_rad2, sinlat1, coslat1, sinlat2, coslat2;
    double londiff, coslondiff, cosdis;

    lat_rad1 = lat1*d2r;
    lat_rad2 = lat2*d2r;

    sinlat1 = sin(lat_rad1);
    coslat1 = cos(lat_rad1);

    sinlat2 = sin(lat_rad2);
    coslat2 = cos(lat_rad2);

    londiff = (lon2 - lon1)*d2r;
    coslondiff = cos( londiff );

    cosdis = sinlat1*sinlat2 + coslat1*coslat2*coslondiff;

    if (cosdis < -1.0) cosdis = -1.0;
    if (cosdis >  1.0) cosdis =  1.0;

    *dis = acos(cosdis);

    *theta = atan2( sin(londiff),
                    sinlat1*coslondiff - coslat1*sinlat2/coslat2 ) - M_PI_2;

}

