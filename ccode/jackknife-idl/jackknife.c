#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "jackknife.h"

/*******************************************************************
 NAME
    jackknife.c

 PURPOSE
    Takes in data array and calculates jackknife statistics
    This is much more efficient than the IDL version.

  This demonstrates the proper way to address the 2-d data array.  
    Much faster than looping imeas,ivar order for large arrays.

   for(ivar=0;ivar<Nvar;ivar++) 
     for(imeas=0;imeas<Nmeas;imeas++)
       tmp = *(data + ivar*Nmeas + imeas);

********************************************************************/

int jackknife(float *data,      /* the data to jackknife [Nmeas, Nvar] */
	      int   Nmeas,          /* Number of measurements of the variables */
	      int   Nvar,           /* Number of variables */
	      int   statistic,      /* What kind of statistic are we jackknifing?*/
	      float *datamean,      /* the mean values */
	      float *covariance)  { /* covariance [Nvar, Nvar] */


    float factor;                 /* factor to get correct variance */
    int   ivar,jvar,imeas,Ntot;              /* for loops over the variables */
    double stot,stot2,jtot,smod,smod2;       /* temporary variable for sums */

    double smean, jmean;          /* mean of sample and sub-samples */
    float tmp;

    float  *sampval;              /* a DAA to to hold the sample values 
                                     [Nmeas, Nvar] */
    float  *jval;                 /* DAA to hold means over subsamples [Nvar] */

    /* allocate temporary arrays */
    Ntot = Nmeas*Nvar;
    if ( (sampval = calloc(Ntot, sizeof(float)) ) == NULL) {
        printf("Error allocating sampval -aborting.");
        return(-1);
    }
    if ( (jval    = calloc(Nvar, sizeof(float)) ) == NULL) {
        printf("Error allocating jval -aborting.");
        free(sampval);
        return(-1);
    }

    factor = (Nmeas-1.)/Nmeas;

    /* loop over the variables */
    for(ivar=0; ivar<Nvar; ivar++) {

        /* initialize */
        jtot = 0.0;

        switch(statistic) {
            case 1:
                { /* Mean */
                    stot = jtotal(data, ivar, Nmeas, 1);
                    smean = stot/Nmeas;
                    break;
                }
            case 2:
                { /* Standard Deviation */
                    stot  = jtotal(data, ivar, Nmeas, 1);
                    stot2 = jtotal(data, ivar, Nmeas, 2);
                    smean = stot/Nmeas;
                    smean = ( stot2 - 2.*smean*stot + Nmeas*smean*smean )/(Nmeas-1.);
                    smean = sqrt(smean);
                    break;
                }
            case 3:
                { /* Variance */
                    stot  = jtotal(data, ivar, Nmeas, 1);
                    stot2 = jtotal(data, ivar, Nmeas, 2);
                    smean = stot/Nmeas;
                    smean = ( stot2 - 2.*smean*stot + Nmeas*smean*smean )/(Nmeas-1.);
                    break;
                }
            default: 
                { 
                    printf("No such statistic \"%d\"\n",statistic);
                    free(jval); free(sampval);
                    return(-1);
                }
        }

        /* loop over the measurements of this variable 
           measure the mean leaving the index imeas out */
        for(imeas=0; imeas<Nmeas; imeas++) {
            switch(statistic) 
            {
                case 1:
                    { /* sample mean */
                        smod = stot - *(data + ivar*Nmeas + imeas);
                        jmean = smod/(Nmeas-1.);
                        *(sampval + ivar*Nmeas + imeas) = jmean;
                        jtot += jmean;
                        break;
                    }
                case 2:
                    { /* sample standard devation */
                        smod  = stot - *(data + ivar*Nmeas + imeas);
                        smod2 = stot2 - 
                            ( *(data + ivar*Nmeas + imeas) )*
                            ( *(data + ivar*Nmeas + imeas) );

                        jmean  = smod/(Nmeas-1.);
                        tmp = 
                            ( smod2 - 2.*jmean*smod + (Nmeas-1)*jmean*jmean )/(Nmeas-2.);
                        tmp = sqrt(tmp);
                        *(sampval + ivar*Nmeas + imeas) = tmp;
                        jtot += tmp;
                        break;
                    }
                case 3:
                    { /* sample variance */
                        smod  = stot - *(data + ivar*Nmeas + imeas);
                        smod2 = stot2 - 
                            ( *(data + ivar*Nmeas + imeas) )*
                            ( *(data + ivar*Nmeas + imeas) );

                        jmean  = smod/(Nmeas-1.);

                        tmp = 
                            ( smod2 - 2.*jmean*smod + (Nmeas-1)*jmean*jmean )/(Nmeas-2.);
                        *(sampval + ivar*Nmeas + imeas) = tmp;
                        jtot += tmp;
                        break;
                    }
                default: 
                    { 
                        printf("No such statistic \"%d\"\n",statistic);
                        free(jval); free(sampval);
                        return(-1);
                    }
            }
        }

        /* 
         *  the mean of this value over these samples 
         */

        *(jval + ivar) = jtot/Nmeas;

        /* the jackknife mean */
        *(datamean + ivar) = smean + (Nmeas-1.)*( smean - *(jval+ivar) );

    }

    /* 
       now go through and calculate the covariance matrix 
       Didn't try to be as efficient as I could; for Nvar << Nmeas
       That's OK 
    */

    for(jvar=0; jvar<Nvar; jvar++) {
        for(ivar=jvar; ivar<Nvar; ivar++) {

            *(covariance + jvar*Nvar + ivar) = 
                factor*jtotalcov(sampval, jval, ivar, jvar, Nmeas);
	  
            if(ivar != jvar) {
                *(covariance + ivar*Nvar + jvar) = 
                    *(covariance + jvar*Nvar + ivar);
            }
	  
        }
    }


    /* free memory */
    free(jval); 
    free(sampval);

    return(0);

}

/*
  Total the elements of data[*, ivar]^power
*/

float jtotal(float *data, int ivar, int Nmeas, int power)

{
  
  register int   imeas; /* register int for loop over measurements */
  float sum=0;  /* the sum for the mean value */

  if (power == 1) 
    {
      for(imeas=0; imeas< Nmeas; imeas++)
	{
	  /* return sum */
	  sum += *(data + ivar*Nmeas + imeas);
	}
    }
  else if (power == 2)
    {
      for(imeas=0; imeas< Nmeas; imeas++)
	{
	  /* return sum of squares */
	  sum += 
	    ( *(data + ivar*Nmeas + imeas) )*( *(data + ivar*Nmeas + imeas) );
	}
    }
  else 
    {
      for(imeas=0; imeas< Nmeas; imeas++)
	{
	  /* return sum */
	  sum += pow( *(data + ivar*Nmeas + imeas), power);
	}
    }

  return(sum);

}
/*
  Calculate the covariance sums
*/
float jtotalcov(float *sampval, float *jval, int ivar, int jvar, int Nsamp)
{

  register int isamp; /* loop over measurements */
  float sum=0;  /* the sum */
  float idiff,jdiff;

  for(isamp=0; isamp<Nsamp; isamp++)
    {
      idiff = *(sampval + ivar*Nsamp + isamp) - *(jval + ivar);
      jdiff = *(sampval + jvar*Nsamp + isamp) - *(jval + jvar);

      sum += idiff*jdiff;
    }

  return(sum);

}
