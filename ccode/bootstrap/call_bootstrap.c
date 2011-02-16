#include <stdlib.h>
#include <stdio.h>
#include "export.h"
#include "bootstrap.h"

/*******************************************************************
 NAME
    call_bootstrap.c

 PURPOSE
    To be called from idl. Takes in data array and sends it to
    the bootstrap program

********************************************************************/

int call_bootstrap(int argc, void *argv[]) 
{

  float *data;                  /* the data to bootstrap [Nmeas, Nvar] */
  int   *tNmeas,Nmeas;          /* Number of measurements of the variables */
  int   *tNvar,Nvar;            /* Number of variables */
  int   *tstatistic,statistic;  /* What kind of statistic are we jackknifing?*/
  float *sampval;               /* Holds the resampled values [Nresamp,Nvar]*/
  int   *tNresamp, Nresamp;     /* Number of samples to create */
  float *datamean;              /* the mean values */
  float *covariance;            /* covariance */

  int   retval;                 /* value to return */

  data = (float *) argv[0];
  tNmeas = (int *) argv[1];
  tNvar = (int *) argv[2];
  tstatistic = (int *) argv[3];
  sampval = (float *) argv[4];
  tNresamp = (int *) argv[5];
  datamean = (float *) argv[6];
  covariance = (float *) argv[7];

  Nmeas = *tNmeas;
  Nvar  = *tNvar;
  statistic = *tstatistic;
  Nresamp = *tNresamp;

  retval = bootstrap(data, Nmeas, Nvar, statistic, sampval, Nresamp, 
		     datamean, covariance);

  return(retval);

}
