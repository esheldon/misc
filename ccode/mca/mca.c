#include <stdlib.h>
#include "mca.h"

void mca_stretch_move(double a,
                      const double *pars, 
                      const double *comp_pars, 
                      size_t npars,
                      double *newpars,
                      double *z)
{
    *z = mca_rand_gofz(a);
    for (size_t i=0; i<npars; i++) {

        double val=pars[i];
        double cval=comp_pars[i];

        newpars[i] = cval + (*z)*(val-cval); 
    }
}

long mca_rand_long(long n)
{
    return lrand48() % n;
}

long mca_rand_complement(long current, long n)
{
    long i=current;
    while (i == current) {
        i = mca_rand_long(n);
    }
    return i;
}

double mca_rand_gofz(double a)
{
    // ( (a-1) rand + 1 )^2 / a;

    double z = (a - 1.)*drand48() + 1.;

    z = z*z/a;

    return z;
}
