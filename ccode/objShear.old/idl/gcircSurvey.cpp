#include "gcircSurvey.h"

// coords in degrees, distance and theta output in radians

int
gcircSurvey(double lam1, double eta1, 
	    double lam2, double eta2,
	    double& dis,  double& theta)

{

  static const float
    D2R=0.017453293,
    R2D=57.295780,
    PI=3.1415927;

  double sinlam1, coslam1, sinlam2, coslam2, etadiff, cosetadiff, cosdis;
  
  sinlam1 = sin(lam1*D2R);
  coslam1 = cos(lam1*D2R);

  sinlam2 = sin(lam2*D2R);
  coslam2 = cos(lam2*D2R);

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
  
  
