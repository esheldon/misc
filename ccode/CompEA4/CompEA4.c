#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "export.h"
#include "CompEA4.h"

int shearmult(double e1a, double e2a, double e1b, double e2b, 
	      double *e1out, double *e2out) 
{
  
  /* This is eq. 2-13 of Bernstein & Jarvis */
  /* Shear ea is applied, then eb -- it matters! */
  double dotp, factor, tmp;
  
  dotp = e1a*e1b + e2a*e2b;

  tmp = 1-e1b*e1b-e2b*e2b;

  if (tmp > 0.0) 
    {
      factor = (1.-sqrt(tmp)) / (e1b*e1b + e2b*e2b);
      
      tmp = 1+dotp;
      
      if (tmp != 0) 
	{
	  *e1out = (e1a + e1b + e2b*factor*(e2a*e1b - e1a*e2b))/(1+dotp);
	  *e2out = (e2a + e2b + e1b*factor*(e1a*e2b - e2a*e1b))/(1+dotp);
	  return(0);
	}
      else
	{
	  *e1out = -9999.;
	  *e2out = -9999.;
	  return(1);
	}
    } 
  else 
    {
      *e1out = -9999.;
      *e2out = -9999.;
      return(1);
    }

}

int CompEA4(double Tratio, 
	    double e1p, double e2p, double a4p, 
	    double e1o, double e2o, double a4o,
	    double *e1, double *e2, double *R) 
{
  
  double e1red, e2red; /* ellipticities reduced to circular PSF */
  double sig2ratio;
  double coshetap, coshetao;
  double e,eta,a2,b2,A,B;
  double Rtmp;
  double a4i;
  double ca4i,ca4p;
  double deltaeta,etai,deltamu;
  double Ti,Tp;
  double EI;
  double tmp1, tmp2;

  /* Take us to sig2ratio = sigma2(P)/sigma2(O) since this is
     shear-invariant */
  tmp1 = 1-e1p*e1p-e2p*e2p;
  tmp2 = 1-e1o*e1o-e2o*e2o;

  if ( (tmp1 <= 0.0) || (tmp2 <= 0.0) ) 
    {
      /* failed sqrt tests #1,#2 above */
      *e1 = DEFVAL;
      *e2 = DEFVAL;
      *R  = DEFVAL;
      return(FLAG_BADE);
    }

  coshetap = 1./sqrt(tmp1);
  coshetao = 1./sqrt(tmp2);
  sig2ratio = Tratio * coshetao/coshetap; /* since sigma2 = T / cosh
					     eta */  
  if (shearmult(e1o,e2o,-e1p,-e2p,&e1red,&e2red) != 0) 
    {
      *e1 = DEFVAL;
      *e2 = DEFVAL;
      *R  = DEFVAL;
      return(FLAG_BADEMULT1);
    }

  /* compute resolution factor and un-dilute */

  tmp1 = e1red*e1red+e2red*e2red;
  if (tmp1 <= 0.0)
    {
      *e1 = DEFVAL;
      *e2 = DEFVAL;
      *R  = DEFVAL;
      return(FLAG_BADERED1);
    }

  e = sqrt(tmp1);
  eta = atanh(e);
  a2 = exp(-eta)*sig2ratio; /* fraction of major axis variance from PSF
			     */
  b2 = exp(eta)*sig2ratio; /* fraction of minor axis variance from PSF
			    */
  A = 1-a2; B = 1-b2; /* fractions from intrinsic image */
  ca4p = 0.375*(a2*a2+b2*b2)+0.25*a2*b2;
  ca4i = 0.375*(A*A+B*B)+0.25*A*B;
  a4i = (a4o - ca4p*a4p) / ca4i;
  Ti = (A-B) * (-2+1.5*(A+B));
  Tp = (a2-b2) * (-2+1.5*(a2+b2));
  deltaeta = Ti * a4i + Tp * a4p;
  
  /* 4th moment correction for R: must find etai */
  EI = e;
  etai = 0.5 * log( (1./a2-1) / (1./b2-1) );

  tmp2 = 1-e1red*e1red-e2red*e2red;

  if (tmp2 <= 0.0)
    {
      *e1 = DEFVAL;
      *e2 = DEFVAL;
      *R  = DEFVAL;
      return(FLAG_BADERED2);
    }

  coshetao = 1./sqrt(tmp2);
  deltamu = (-1.5*A*A - A*B - 1.5*B*B +2*(A+B)) * a4i + 
    (-1.5*a2*a2 - a2*b2 - 1.5*b2*b2 + 2*(a2+b2))*a4p;
  deltamu *= 0.5;
  deltaeta *= -1.0;
  Rtmp = ( 1 - 2*deltamu - deltaeta*EI - sig2ratio/coshetao ) / (-deltaeta/EI + 1-2*deltamu ) ;
  
  /* Don't divide by zero */
  if (Rtmp == 0.0) 
    {
      *e1 = DEFVAL;
      *e2 = DEFVAL;
      *R  = DEFVAL;
      return(FLAG_BADRVAL);
    }

  e1red /= Rtmp;
  e2red /= Rtmp;
      
  if (shearmult(e1red,e2red,e1p,e2p,e1,e2) != 0)
    {
      *e1 = DEFVAL;
      *e2 = DEFVAL;
      *R  = DEFVAL;
      return(FLAG_BADEMULT2);
    }
  
  *R = Rtmp;
  
  return(0);
  
}

