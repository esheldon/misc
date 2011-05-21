#include "lens_constants.h"
#include "gcirc.h"
#include "math.h"

/*
 *
 * lat [-90,90]
 * lon [0,360] and [-180,180]  I think both will work since the
 * longitude comes in only as a diff.
 *
 * We will have to figure out our transformation from pixels to this
 * coord system
 */
void gcirc(double lon1, double lat1, 
           double lon2, double lat2, 
           double& dis, double& theta) {


    double lat_rad1 = lat1*D2R;
    double lat_rad2 = lat2*D2R;

    double sinlat1 = sin(lat_rad1);
    double coslat1 = cos(lat_rad1);

    double sinlat2 = sin(lat_rad2);
    double coslat2 = cos(lat_rad2);

    // should we reverse this to lon2-lon1?  This produced
    // a minus sign in the angle theta below
    double londiff = (lon2 - lon1)*D2R;
    double coslondiff = cos( londiff );

    double cosdis = sinlat1*sinlat2 + coslat1*coslat2*coslondiff;

    if (cosdis < -1.0) cosdis = -1.0;
    if (cosdis >  1.0) cosdis =  1.0;

    dis = acos(cosdis);

    theta = atan2( sin(londiff), 
                   sinlat1*coslondiff - coslat1*sinlat2/coslat2 ) - M_PI_2;
}
 
void gcirc_eq(double ra1, double dec1, 
           double ra2, double dec2, 
           double& dis, double& theta) {


    double dec_rad1 = dec1*D2R;
    double dec_rad2 = dec2*D2R;

    double sindec1 = sin(dec_rad1);
    double cosdec1 = cos(dec_rad1);

    double sindec2 = sin(dec_rad2);
    double cosdec2 = cos(dec_rad2);

    double radiff = (ra1 - ra2)*D2R;
    double cosradiff = cos( radiff );

    double cosdis = sindec1*sindec2 + cosdec1*cosdec2*cosradiff;

    if (cosdis < -1.0) cosdis = -1.0;
    if (cosdis >  1.0) cosdis =  1.0;

    dis = acos(cosdis);

    theta = atan2( sin(radiff), 
                   sindec1*cosradiff - cosdec1*sindec2/cosdec2 ) - M_PI;
}
 
/*
 * The returned angle theta works properly with how I transformed from
 * pixels to lambda,eta in the SDSS
 */
void gcirc_survey(
        double lam1, double eta1, 
        double lam2, double eta2,
        double& dis,  double& theta)
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

  dis = acos(cosdis);

  theta = atan2( sinetadiff, 
  		 (sinlam1*cosetadiff - coslam1*sinlam2/coslam2) ) - M_PI_2;

}


