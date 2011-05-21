#include "sigmaCritInv.h"
#include "angDist.h"

using namespace std;




/*
 * return in inverse solar masses per square parsecs
 */

float sigmaCritInv(float H0, float omegaMatter, float zLens, float zSource)
{

  static const float BADVAL=-1.0;

  float DLS, DL, DS, D, sig_inv;

  DLS = angDist(H0, omegaMatter, zLens, zSource);
  DL  = angDist(H0, omegaMatter, zLens);
  DS  = angDist(H0, omegaMatter, zSource);

  // Megaparsecs
  D = DLS * DL/DS;  

  // Gpc
  D = D*0.001; 

  sig_inv = D/1.663e3;
  if (sig_inv < 0) sig_inv = BADVAL;
  
  return(sig_inv);

}

// get the inverse critical density using the aeta conformal time from
// Pen 1997
//
// see esutil/cosmology.py Cosmo class for the constant
//   four_pi_over_c_squared(dunits='Mpc')
//
// units: inverse solar masses per square parsecs
//
// here  aeta_rel is 
//     aeta(0) - aeta(z)
//
// multiply the result by the distance to the lens to get the proper
// value.

// DL in Mpc

float sigmaCritInv(float DL, float aeta_rel_lens, float aeta_rel_source) {
    
    static const float four_pi_over_c_squared = 6.0150504541630152e-07;

    float retval = (aeta_rel_source - aeta_rel_lens)/aeta_rel_source;
    retval *= four_pi_over_c_squared*DL;

    return retval;
}







float 
sigmaCritInvInterp3d(float& zLens, float& zSource, float& zSourceErr, 
	     struct scinv* sc_inv)
{


  float fzl, fzs, fzsErr;
  float sig_inv;

  fzsErr = (zSourceErr - sc_inv->zsErrMin)/sc_inv->zsErrStep;
  fzs    = (zSource    - sc_inv->zsMin)   /sc_inv->zsStep;
  fzl    = (zLens      - sc_inv->zlMin)   /sc_inv->zlStep;

  sig_inv = (float) interpolateScinvStruct3d(fzsErr, fzs, fzl, sc_inv);

  return(sig_inv);
}

// Interpolate the 3d scinvStruct
// values outside range not allowed, BADVAL is returned
double 
interpolateScinvStruct3d(float fzsErr, float fzs, float fzl,
		       struct scinv* s)            
{

  static const float BADVAL=-1.0;

  int 
    x0, y0, z0,
    x1, y1, z1;
  double Vxyz;

  float x,y,z;

  // initialization and range checking
  x0 = (int) fzsErr;
  if (x0 < 0) return(BADVAL);
  y0 = (int) fzs;
  if (y0 < 0) return(BADVAL); 
  z0 = (int) fzl;
  if (z0 < 0) return(BADVAL);

  x1 = x0+1;
  if (x1 > (NZSERR-1))  return(BADVAL);
  y1 = y0+1;
  if (y1 > (NZS-1))     return(BADVAL);
  z1 = z0+1;
  if (z1 > (NZL-1))     return(BADVAL);

  /* x,y,z in cube */
  x=fzsErr-x0;
  y=fzs-y0;
  z=fzl-z0;

  Vxyz = 	
    s->scinv[x0][y0][z0]*(1 - x)*(1 - y)*(1 - z) +
    s->scinv[x1][y0][z0]*x*(1 - y)*(1 - z) +
    s->scinv[x0][y1][z0]*(1 - x)*y*(1 - z) +
    s->scinv[x0][y0][z1]*(1 - x)*(1 - y)*z +
    s->scinv[x1][y0][z1]*x*(1 - y)*z +
    s->scinv[x0][y1][z1]*(1 - x)*y*z +
    s->scinv[x1][y1][z0]*x*y*(1 - z) +
    s->scinv[x1][y1][z1]*x*y*z;

  return(Vxyz);

} 

float sigmaCritInvInterp2d(
        float& zLens, 
        float& zSource, 
        struct scinv2d *sc_inv)
{


  float fzl, fzs;
  float sig_inv;

  fzl    = (zLens   - sc_inv->zlMin)/sc_inv->zlStep;
  fzs    = (zSource - sc_inv->zsMin)/sc_inv->zsStep;

  sig_inv = (float) interpolateScinvStruct2d(fzl, fzs, sc_inv);

  return(sig_inv);
}

// Interpolate the 2d scinv struct
// values outside range not allowed, BADVAL is returned
double interpolateScinvStruct2d(
        float fzl, 
        float fzs,
        struct scinv2d *s)            
{

  static const float BADVAL=-1.0;

  int 
    x0, y0,
    x1, y1;
  double Vxy;

  float x,y;

  // initialization and range checking
  x0 = (int) fzl;
  if (x0 < 0) 
      return(BADVAL);
  y0 = (int) fzs;
  if (y0 < 0) 
      return(BADVAL); 

  x1 = x0+1;
  if (x1 > (s->nzl-1))  
      return(BADVAL);
  y1 = y0+1;
  if (y1 > (s->nzs-1))  
      return(BADVAL);

  /* x,y,z in cube */
  x=fzl-x0;
  y=fzs-y0;

  Vxy = 
      s->scinv[x0*s->nzl + y0]*(1 - x)*(1 - y) + 
      s->scinv[x1*s->nzl + y0]*x*(1 - y)       + 
      s->scinv[x0*s->nzl + y1]*(1-x)*y         + 
      s->scinv[x1*s->nzl + y1]*x*y;

  return(Vxy);

} 




