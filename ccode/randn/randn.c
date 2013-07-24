#include <stdlib.h>
#include <time.h>
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

    The small lambda one is from Knuth

    The cut/rejection method is based off numerical recipes
*/
long poisson(double lambda)
{
    long k=0;

    if (lambda <= 0) {
        k=0;
    } else if (lambda > 12) {
        // use cut method, based on numerical recipes
        double sq=sqrt(2*lambda);
        double loglam=log(lambda);
        double em=0;
        while (1) {
            double y=tan(M_PI*drand48());
            em=sq*y+lambda;

            if (em < 0.0) {
                continue;
            }

            // take integer part
            em=(double) ( (long) em );
            double g=lambda*loglam-lgamma(lambda+1.);
            double t=0.9*(1.+y*y)*exp(em*loglam-lgamma(em+1.)-g);

            if (drand48() <= t) {
                break;
            }
        }

        // em is already the integer part
        k = (long) em;
    } else {
        // this works great but for high values of lambda (lambda > 700),
        // exp(-lambda) is returned as exactly zero
        double p=drand48();
        double target=exp(-lambda);
        while (p > target) {
            p*=drand48();
            k+=1;
        }
    }
    return k;
}

// random numbers in the range [-1,1]
double srandu()
{
    return 2*(drand48()-0.5);
}


void randn_seed(void)
{
    time_t t1;
    (void) time(&t1);
    srand48((long) t1);
}
