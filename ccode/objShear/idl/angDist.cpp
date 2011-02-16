#include "angDist.h"

//////////////////////////////////////////
// Assumes flat universe.  
//////////////////////////////////////////

// Conformal time
float
aeta(float& z, float& omega_m)
{

  float 
    s, s2, s3, s4, 
    a, a2, a3, a4,
    time;

  static const float 
    c0=1.0, c1= -.1540, c2=.4304, c3= .19097, c4=.066941, exp= -0.125;

  s3 = (1.-omega_m)/omega_m;
  s = pow(s3, 0.333333);
  s2 = s*s;
  s4 = s2*s2;
  a = 1./(1.0+z);

  a2 = a*a;
  a3 = a2*a;
  a4 = a3*a;

  time = c0/a4 + c1*s/a3 + c2*s2/a2 + c3*s3/a + c4*s4;
  time = 2.0*sqrt(s3+1.0)*pow(time, exp);
  return(time);

}


// Overloaded: this one calculates the andgist between zero and input redshift

float
angDist(float& H0, float& omega_m, float& z)
{

  static float z_zero=0;
  static const float c=2.9979e5;

  float 
    fac,
    onePlusZ,
    lum_dist, ang_dist;
  
  onePlusZ = 1.0+z;

  fac = c/H0*onePlusZ;
  
  lum_dist = fac*( aeta(z_zero, omega_m) - aeta(z, omega_m) );
  ang_dist = lum_dist/pow(onePlusZ, 2);

  return(ang_dist);
}


// Overloaded: This one takes in a minimum redshift
float
angDist(float& H0, float& omega_m, float& zmin, float& zmax)
{

  static const float c=2.9979e5;

  float 
    fac,
    onePlusZmax,
    lum_dist, ang_dist;
  
  onePlusZmax = 1.0+zmax;

  fac = c/H0*onePlusZmax;
  
  lum_dist = fac*( aeta(zmin, omega_m) - aeta(zmax, omega_m) );
  ang_dist = lum_dist/pow(onePlusZmax, 2);

  return(ang_dist);
}

