#include <stdlib.h>
#include <math.h>
#include "randn.h"

/*
  Note we get two per run but I'm only using one.
*/
double randn() 
{
    double x1, x2, w, y1;//, y2;
 
    do {
        x1 = 2.*drand48() - 1.0;
        x2 = 2.*drand48() - 1.0;
        w = x1*x1 + x2*x2;
    } while ( w >= 1.0 );

    w = sqrt( (-2.*log( w ) ) / w );
    y1 = x1*w;
    //y2 = x2*w;
    return y1;
}

/*
    from Knuth
*/
long poisson(double lambda)
{
    long k=0;
    double target=exp(-lambda);
    double p=drand48();

    while (p > target) {
        p*=drand48();
        k+=1;
    }

    return k;
}
