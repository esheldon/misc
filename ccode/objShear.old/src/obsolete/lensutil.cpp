#include "lensutil.h"

using namespace std;

void
objShearPrintMeans(int lensIndex, int nlens, float sigsum, float sigwsum)
{

  float meanDenscont;
  float meanDenscontErr;

  if (sigwsum > 0) 
    {
      meanDenscont = sigsum/sigwsum;
      meanDenscontErr = sqrt(1.0/sigwsum);
      printf("\nlens = %d/%d   Mean dens. cont. = %f +/- %f\n",
	     lensIndex,nlens,
	     meanDenscont,meanDenscontErr);		    
    }
  else 
    {
      printf("\nlens = %d/%d   Mean dens. cont. = 0 +/- 0\n",
	     lensIndex,nlens);		    
    }
}

int
objShearTestQuad(int16& bad12, 
		 int16& bad23, 
		 int16& bad34,
		 int16& bad41, 
		 float64& theta)
{
   
  static const int UNMASKED=1, MASKED=0;
  static const float 
    PI=3.1415927,
    TWOPI=6.2831853,
    PIOVER2=1.5707963,
    THREE_PIOVER2=4.7123890;

  // 1+2 or 3+4 are not masked
  if ( !bad12 || !bad34 ) 
    {

      // keeping both quadrants
      if ( !bad12 && !bad34 )
	{
	  return(UNMASKED);
	}
      
      // only keeping one set of quadrants
      if (!bad12)
	{
	  if (theta >= 0.0 && theta <= PI)
	    return(UNMASKED);
	  else
	    return(MASKED);
	}
      else
	{
	  if (theta >= PI && theta <= TWOPI)
	    return(UNMASKED);
	  else
	    return(MASKED);
	}


    }

  // 2+3 or 4+1 are not masked
  if ( !bad23 || !bad41 ) 
    {

      // keeping both quadrants
      if ( !bad23 && !bad41 )
	{
	  return(UNMASKED);
	}
      
      // only keeping one set of quadrants
      if (!bad23)
	{
	  if (theta >= PIOVER2 && theta <= THREE_PIOVER2)
	    return(UNMASKED);
	  else
	    return(MASKED);
	}
      else
	{
	  if ( (theta >= THREE_PIOVER2 && theta <= TWOPI) ||
	       (theta >= 0.0           && theta <= PIOVER2) )
	    return(UNMASKED);
	  else
	    return(MASKED);
	}

    }


}
