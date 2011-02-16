#if !defined (_lensutil_h)
#define _lensutil_h

#include "types.h"
#include "lens_constants.h"
#include <stdio.h>
#include <math.h>
void
objShearPrintMeans(int lensIndex, int nlens, float sigsum, float sigwsum);
int
objShearTestQuad(int16& bad12, 
		 int16& bad23, 
		 int16& bad34,
		 int16& bad41, 
		 float64& theta);

#endif /* _lensutil_h */
