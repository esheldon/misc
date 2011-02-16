#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include "bootstrap.h"

/*******************************************************************
  NAME
  bootstrap.c

  PURPOSE
  Takes in data array and calculates bootstrap statistics
  This isn't actually much more efficient than the IDL version
  for a ralatively small Nmeas (e.g. Nvar=2, Nmeas=5000) but
  for larger it is: Nmeas=80000 its a factor of 2 faster

  This demonstrates the proper way to address the 2-d data array.  
  Faster than looping imeas,ivar order for large arrays.

  for(ivar=0;ivar<Nvar;ivar++) 
  for(imeas=0;imeas<Nmeas;imeas++)
  tmp = *(data + ivar*Nmeas + imeas);

 ********************************************************************/

int bootstrap(float *data,     /* the data to bootstrap [Nmeas, Nvar] */
        int   Nmeas,     /* Number of measurements of the variables */
        int   Nvar,      /* Number of variables */
        int   statistic, /* What kind of statistic? */
        float *sampval,  /* an array to hold the resampled values 
                            [Nresamp, Nvar] */
        int   Nresamp,   /* Number of random samples to create */
        float *datamean, /* the mean values */
        float *covariance)  /* covariance [Nvar, Nvar] */

{

    int   ivar,jvar;              /* for loops over the variables */
    float jmean, imean;           /* temporary variable for means */
    int   retval;

    /* resample the data array */

    retval = bootstrap_resample(data, Nmeas, Nvar, statistic, Nresamp, sampval);

    if (retval != 0) return(retval);

    /* loop over the variables and get mean of this statistic */
    for(ivar=0; ivar<Nvar; ivar++) 
    {
        *(datamean + ivar) = bmean(sampval, ivar, Nresamp);
    }

    /* Now calculate covariance */
    for(jvar=0; jvar<Nvar; jvar++)
    {
        jmean = *(datamean + jvar);
        for(ivar=jvar; ivar<Nvar; ivar++)
        {
            imean = *(datamean + ivar);
            *(covariance + jvar*Nvar + ivar) = 
                bcov(sampval, jvar, jmean, ivar, imean, Nresamp);

            if(ivar != jvar) 
            {
                *(covariance + ivar*Nvar + jvar) = 
                    *(covariance + jvar*Nvar + ivar);
            }

        }
    }
    return(0);

}

/* 
   create random samples for bootstrapping 
   */
int bootstrap_resample(float *data, /* data to be resamples [Nmeas,Nvar]*/
        int Nmeas,   
        int Nvar, 
        int statistic, /* which statistic to measure? */ 
        int Nresamp,   /* Number of samples */
        float *sampval) /* the values for samples */
{

    int irand; /* random array index */
    int *randarray; /* array of random indices */
    int imeas, ivar, isamp; /* loop over Nmeas and Nvar and Nresamp */
    int   stime;
    long  ltime; /* time for seeding rand() */
    float sum;

    if ( (randarray = calloc(Nmeas, sizeof(int)) ) == NULL )
    {
        printf("Error allocating randarray.");
        return(-1);
    }

    /* seed the random number generator */
    ltime = time(NULL);
    stime = (unsigned) ltime/2;
    srand(stime);
    /* 
       we use the same random indices for each variable, since they
       are assumed to be related (otherwise why calculate covariance?)
       */
    for(isamp=0;isamp<Nresamp;isamp++)
    {
        /* generate an array of random indices */
        for(imeas=0;imeas<Nmeas;imeas++)
        {
            /* create random indices from [0, Nmeas-1] */
            /* THIS MIGHT BE FASTER IF WE ALSO SORTED RANDARRAY */
            irand = rand();
            irand = (int) ( ( (float) irand )/RAND_MAX*Nmeas );

            /* Might actually equal Nmeas in some rare case */
            if (irand == Nmeas) irand=Nmeas-1;

            *(randarray + imeas) = irand;
        } /* Over Nmeas */

        /* now, for each variable, measure statistic for this sample */
        for(ivar=0;ivar<Nvar;ivar++)
        {
            sum = 0.0;
            for(imeas=0;imeas<Nmeas;imeas++)
            {
                irand = *(randarray + imeas);
                /* which statistic? */
                /* for now must mean */
                sum += *(data + ivar*Nmeas + irand);
            } /* Over Nmeas */

            *(sampval + ivar*Nresamp + isamp) = sum/Nmeas;

        } /* Over Nvar */
    } /* Over Nresamp */

    free(randarray);
    return(0);

} /* end bootstrap_resamp */

/* find the mean */
float bmean(float *data, int ivar, int Nresamp)

{

    register int isamp; /* register int for loop over samples */
    float sum=0;  /* the sum for the mean value */

    for(isamp=0;isamp<Nresamp;isamp++) sum += *(data + ivar*Nresamp + isamp);
    return(sum/Nresamp);

}

/* find the covariance for input variables */
float bcov(float *data, 
        int jvar, float jmean, 
        int ivar, float imean, 
        int Nresamp)

{

    register int   isamp; /* register int for loop over samples */
    float sum=0;  /* the sum for the mean value */
    float jtmp, itmp;

    /* first find mean */
    /* assume here the outind is in the range unless
       it equals -1 */

    for(isamp=0; isamp< Nresamp; isamp++)
    {
        jtmp = *(data + jvar*Nresamp + isamp) - jmean;
        itmp = *(data + ivar*Nresamp + isamp) - imean;

        sum += jtmp*itmp;
    }
    return(sum/(Nresamp-1.));

}
