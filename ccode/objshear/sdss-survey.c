#include <math.h>
#include "defs.h"

double lon_bound(double lon, double minval, double maxval) {
    while (lon < minval) {
        lon += 360.;
    }

    while (lon > maxval) {
        lon -= 360.;
    }
    return lon;
}

/*
 * ra,dec input in degrees
 * lam,eta output in degrees
 */
void eq2sdss(double ra, double dec, double* lam, double* eta) {

    // put everything in radians first
    ra *= D2R;
    dec *= D2R;

    // this is the SDSS node
    ra -= 1.6580627893946132;

    double cdec = cos(dec);
    
    double x = cos(ra)*cdec;
    double y = sin(ra)*cdec;
    double z = sin(dec);

    *lam = -asin(x);
    *eta = atan2(z, y);

    // this is the eta pole
    *eta -= 0.56723200689815712;

    *eta *= R2D;
    *lam *= R2D;

    *eta = lon_bound(*eta, -180., 180.);
}

void eq2sdss_sincos(double ra, double dec, 
                    double* sinlam, double* coslam, 
                    double* sineta, double* coseta) {
    double lam,eta;
    eq2sdss(ra, dec, &lam, &eta);

    lam *= D2R;
    eta *= D2R;
    *sinlam = sin(lam);
    *coslam = cos(lam);
    *sineta = sin(eta);
    *coseta = cos(eta);
}
