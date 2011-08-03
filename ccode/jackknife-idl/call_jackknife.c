#include <stdlib.h>
#include <stdio.h>
#include "export.h"
#include "jackknife.h"

/*******************************************************************
 NAME
    call_jackknife.c

 PURPOSE
    To be called from idl. Takes in data array and sends it to
    the jackknife program

********************************************************************/

int call_jackknife(int argc, void *argv[]) 
{
  
  float *data;                  /* the data to jackknife [Nmeas, Nvar] */
  int   *tNmeas,Nmeas;          /* Number of measurements of the variables */
  int   *tNvar,Nvar;            /* Number of variables */
  int   *tstatistic,statistic;  /* What kind of statistic are we jackknifing?*/
  float *datamean;              /* the mean values */
  float *covariance;            /* covariance */

  int   retval;                 /* value to return */

  data = (float *) argv[0];
  tNmeas = (int *) argv[1];
  tNvar = (int *) argv[2];
  tstatistic = (int *) argv[3];
  datamean = (float *) argv[4];
  covariance = (float *) argv[5];

  Nmeas = *tNmeas;
  Nvar  = *tNvar;
  statistic = *tstatistic;
  
  retval = jackknife(data, Nmeas, Nvar, statistic, 
		     datamean, covariance);

  return(retval);

}
