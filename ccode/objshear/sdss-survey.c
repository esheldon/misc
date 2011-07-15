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

/*
 * The returned angle theta works properly with how I transformed from
 * pixels to lambda,eta in the SDSS
 */
void gcirc_survey(
        double lam1, double eta1, 
        double lam2, double eta2,
        double* dis,  double* theta)
{

  double tlam1 = lam1*D2R;
  //sinlam1 = sin(tlam1);
  double coslam1 = cos(tlam1);
  double sinlam1 = sqrt(1.0-coslam1*coslam1);
  if (tlam1 < 0) sinlam1 = -sinlam1;

  double tlam2 = lam2*D2R;
  //sinlam2 = sin(tlam2);
  double coslam2 = cos(tlam2);
  double sinlam2 = sqrt(1.0-coslam2*coslam2);
  if (tlam2 < 0) sinlam2 = -sinlam2;

  double etadiff = (eta2-eta1)*D2R;
  double cosetadiff = cos(etadiff);
  double sinetadiff = sqrt(1.0-cosetadiff*cosetadiff);
  if (etadiff < 0) sinetadiff=-sinetadiff;

  double cosdis = sinlam1*sinlam2 + coslam1*coslam2*cosetadiff;

  if (cosdis < -1.0) cosdis=-1.0;
  if (cosdis >  1.0) cosdis= 1.0;

  *dis = acos(cosdis);

  *theta = atan2( sinetadiff, 
  		 (sinlam1*cosetadiff - coslam1*sinlam2/coslam2) ) - M_PI_2;

}


// convert the theta returned from gcirc_survey into a quadrant
// as used in stomp
int gcirc_theta_quad(double theta) {
    theta = (M_PI_2 - theta)*R2D;

    if (theta >= 0. && theta < 90.) {
        return 0;
    } else if (theta >= 90. && theta < 180.) {
        return 1;
    } else if (theta >= 180. && theta < 270.) {
        return 2;
    } else {
        return 3;
    }
}
