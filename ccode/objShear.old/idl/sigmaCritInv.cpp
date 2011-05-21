#include "objShear.h"
#include "sigmaCritInv.h"
#include "angDist.h"


/////////////////////////////////////////////////////////////////////////
// sigmaCritInv
//
// This function is overloaded.
//
// This one interpolates the intergral over the redshift distribution
/////////////////////////////////////////////////////////////////////////

float 
sigmaCritInv(float& zLens, float& zSource, float& zSourceErr, 
	     SCINV_STRUCT *scinvStruct)
{


  float fzl, fzs, fzsErr;
  float sig_inv;

  fzsErr = (zSourceErr - scinvStruct->zsErrMin)/scinvStruct->zsErrStep;
  fzs    = (zSource    - scinvStruct->zsMin)   /scinvStruct->zsStep;
  fzl    = (zLens      - scinvStruct->zlMin)   /scinvStruct->zlStep;

  sig_inv = (float) interpolateScinvStruct(fzsErr, fzs, fzl, scinvStruct);

  return(sig_inv);
}

// Interpolate the 3d scinvStruct
// values outside range not allowed, BADVAL is returned
double 
interpolateScinvStruct(float fzsErr, float fzs, float fzl,
		       SCINV_STRUCT *s)            
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


////////////////////////////////////////////////////////////
// just return in inverse solar masses per square parsecs
////////////////////////////////////////////////////////////

float
sigmaCritInv(float& H0, float& omegaMatter, float& zLens, float& zSource)
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










