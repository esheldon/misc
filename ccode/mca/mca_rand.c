#include <stdlib.h>
#include "mca_rand.h"

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
