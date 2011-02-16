#include <stdlib.h>
#include <stdio.h>

#include "idl_export.h"

int test_autoglue(double* arr, double* sumarr, IDL_LONG* num)
{
  int i;
  double runsum=0;

  for (i=0; i<*num; i++)
  {
    runsum+=arr[i];
    sumarr[i] = runsum;
  }
  return(0);
}
