#include <stdlib.h>
#include <stdio.h>
#include "export.h"
#include "gauleg.h"

int call_gauleg(int argc, void *argv[])
{

  double *x1, *x2, *x, *w;
  int *npts;

  x1 = (double *) argv[0];
  x2 = (double *) argv[1];
  npts = (int *) argv[2];
  x = (double *) argv[3];
  w = (double *) argv[4];


  gauleg(*x1, *x2, *npts, x, w);

}
