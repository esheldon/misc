#include <stdio.h>
#include <stdlib.h>
#include "arrgen.h"

/* indgen function */

int *indgen(long n) {
  
  int *arr;
  long i;

  arr = calloc(n, sizeof(int));

  if ( !( arr = malloc(n*sizeof(int)) ) )
    {
      return(NULL);
    }

  for (i=0;i<n;++i) arr[i] = i;

  return arr;

}

/* lindgen function */

long *lindgen(long n) {
  
  long *arr;
  long i;

  if ( !( arr = malloc(n*sizeof(long)) ) )
    {
      return(NULL);
    }

  for (i=0;i<n;++i) arr[i] = i;

  return arr;

}

/* findgen function */

float *findgen(long n) {
  
  float *arr;
  long i;

  if ( !( arr = malloc(n*sizeof(float)) ) )
    {
      return(NULL);
    }

  for (i=0;i<n;++i) arr[i] = i;

  return arr;

}

/* dindgen function */

double *dindgen(long n) {
  
  double *arr;
  long i;

  if ( !( arr = malloc(n*sizeof(double)) ) )
    {
      return(NULL);
    }

  for (i=0;i<n;++i) arr[i] = i;

  return arr;

}
