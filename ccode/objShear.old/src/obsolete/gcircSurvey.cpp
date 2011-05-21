#include "gcircSurvey.h"

using namespace std;
 
// coords in degrees, distance and theta output in radians

int
gcircSurvey(double lam1, double eta1, 
	    double lam2, double eta2,
	    double& dis,  double& theta)

{

  static const double
    D2R=0.017453293,
    R2D=57.295780,
    PI=3.1415927;

  double sinlam1, coslam1, sinlam2, coslam2, 
    etadiff, cosetadiff, cosdis;
  double tlam1, tlam2;

  tlam1 = lam1*D2R;
  sinlam1 = sin(tlam1);
  coslam1 = cos(tlam1);

  tlam2 = lam2*D2R;
  sinlam2 = sin(tlam2);
  coslam2 = cos(tlam2);

  etadiff = (eta2-eta1)*D2R;
  cosetadiff = cos(etadiff);

  cosdis = sinlam1*sinlam2 + coslam1*coslam2*cosetadiff;

  if (cosdis < -1.0) cosdis=-1.0;
  if (cosdis >  1.0) cosdis= 1.0;

  dis = acos(cosdis);

  theta = atan2( sin(etadiff), 
  		 (sinlam1*cosetadiff - coslam1*sinlam2/coslam2) ) - PI/2.;

  return(0);

}
  
  
int
gcircSurvey2(double lam1, double eta1, 
	     double lam2, double eta2,
	     double& dis,  double& theta)

{

  static const double
    D2R=0.017453293,
    R2D=57.295780,
    PI=3.1415927;

  double sinlam1, coslam1, sinlam2, coslam2, 
    etadiff, cosetadiff, sinetadiff, cosdis;
  double tlam1, tlam2;

  tlam1 = lam1*D2R;
  //sinlam1 = sin(tlam1);
  coslam1 = cos(tlam1);
  sinlam1 = sqrt(1.0-coslam1*coslam1);
  if (tlam1 < 0) sinlam1 = -sinlam1;

  tlam2 = lam2*D2R;
  //sinlam2 = sin(tlam2);
  coslam2 = cos(tlam2);
  sinlam2 = sqrt(1.0-coslam2*coslam2);
  if (tlam2 < 0) sinlam2 = -sinlam2;

  etadiff = (eta2-eta1)*D2R;
  cosetadiff = cos(etadiff);
  sinetadiff = sqrt(1.0-cosetadiff*cosetadiff);
  if (etadiff < 0) sinetadiff=-sinetadiff;

  cosdis = sinlam1*sinlam2 + coslam1*coslam2*cosetadiff;

  if (cosdis < -1.0) cosdis=-1.0;
  if (cosdis >  1.0) cosdis= 1.0;

  dis = acos(cosdis);

  theta = atan2( sinetadiff, 
  		 (sinlam1*cosetadiff - coslam1*sinlam2/coslam2) ) - PI/2.;

  return(0);

}
  
  
